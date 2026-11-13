/* This file is part of Racdoor.
   Copyright (c) 2024 chaoticgd. All rights reversed.
   Released under the BSD-1-Clause license. */

#include <racdoor/crypto.h>
#include <racdoor/elf.h>
#include <racdoor/exploit.h>
#include <racdoor/mips.h>
#include <racdoor/save.h>
#include <racdoor/util.h>

#include <string.h>
#include <time.h>

void inject_rac(SaveSlot* save, Buffer rdx, u32 key);
static void resetUyaHelp(const char* input_path, const char* output_path);

int main(int argc, char** argv)
{
	if (argc == 4 && strcmp(argv[1], "--reset-uya-help") == 0) {
		resetUyaHelp(argv[2], argv[3]);
		return 0;
	}

	const char* input_save_path = NULL;
	const char* input_rdx_path = NULL;
	const char* output_save_path = NULL;
	u32 key = 0;
	
	/* Parse the command line arguments. */
	u32 position = 0;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-k") == 0)
		{
			i++;
			CHECK(i < argc, "Expected key.");
			key = strtoll(argv[i], NULL, 0);
		}
		else
		{
			switch (position++)
			{
				case 0: input_save_path = argv[i]; break;
				case 1: input_rdx_path = argv[i]; break;
				case 2: output_save_path = argv[i]; break;
				default: input_save_path = NULL; /* Print usage message. */
			}
		}
	}
	
	CHECK(input_save_path && input_rdx_path && output_save_path,
		"usage: %s [-k <key>] <input saveN.bin> <input rdx> <output saveN.bin>\n"
		"       %s --reset-uya-help <input saveN.bin> <output saveN.bin>",
		argc > 0 ? argv[0] : "inject", argc > 0 ? argv[0] : "inject");
	
	Buffer file = read_file(input_save_path);
	SaveSlot save = parse_save(file);
	
	Buffer rdx = read_file(input_rdx_path);
	
	switch (save.game.block_count)
	{
		case RAC_GAME_BLOCK_COUNT:
			inject_rac(&save, rdx, key);
			break;
		case GC_GAME_BLOCK_COUNT:
			printf("Going Commando support not yet implemented.\n");
			break;
		case UYA_GAME_BLOCK_COUNT:
			ERROR("Up Your Arsenal bootstrap is not implemented; no output save was written.");
		case DL_GAME_BLOCK_COUNT:
			printf("Deadlocked support not yet implemented.\n");
			break;
		default:
			ERROR("Unexpected block count.");
	}
	
	update_checksums(&save);
	write_file(output_save_path, file);
}

/* Reset the secondary QuickSelect tutorial on an ordinary UYA save.
   Message IDs are independent of the 12-byte help-record indices. */
static void resetUyaHelp(const char* input_path, const char* output_path)
{
	CHECK(strcmp(input_path, output_path) != 0, "Use a separate output save.");
	Buffer file = read_file(input_path);
	SaveSlot save = parse_save(file);
	CHECK(save.game.block_count == UYA_GAME_BLOCK_COUNT
		&& save.level_count == 37, "Expected a UYA save with 37 level sections.");

	SaveBlock* messages = lookup_block(&save.game, BLOCK_HelpDataMessages);
	SaveBlock* flags = lookup_block(&save.game, 5);
	SaveBlock* history = lookup_block(&save.game, BLOCK_HelpLog);
	SaveBlock* position = lookup_block(&save.game, BLOCK_HelpLogPos);
	CHECK(messages->size == 356 * 12 && flags->size == 0x110
		&& history->size == 400 * 2 && position->size == 4,
		"Unexpected UYA help-block layout.");

	s32 count;
	memcpy(&count, position->data, sizeof(count));
	CHECK(count >= 0 && count <= 400,
		"Expected an ordinary in-bounds help history, not an armed exploit.");

	/* Keep unrelated messages and their order; clear only the vacated tail. */
	s32 kept = 0;
	for (s32 i = 0; i < count; i++) {
		u16 id;
		memcpy(&id, history->data + i * 2, sizeof(id));
		if (id == 0x14fb || id == 0x14fc)
			continue;
		memcpy(history->data + kept * 2, &id, sizeof(id));
		kept++;
	}
	memset(history->data + kept * 2, 0, (count - kept) * 2);
	memcpy(position->data, &kept, sizeof(kept));

	/* Clear count/time/planet state for both camera-specific records and
	   the independent "secondary QuickSelect help shown" bit. */
	memset(messages->data + 0x11e * 12, 0, 2 * 12);
	flags->data[0xdf] &= ~4;
	update_checksums(&save);
	write_file(output_path, file);
	printf("Reset UYA QuickSelect help; removed %d history entries. No payload installed.\n",
		count - kept);
}

void inject_rac(SaveSlot* save, Buffer rdx, u32 key)
{
	/* Make sure the player has more than 8 gadgets unlocked. */
	SaveBlock* unlocks = lookup_block(&save->game, BLOCK_Unlocks);
	
	u32 gadgets_unlocked = 0;
	for (u32 i = 0; i < unlocks->size; i++)
		if (unlocks->data[i])
			gadgets_unlocked++;
	
	CHECK(gadgets_unlocked > 8,
		"A save file with more than 8 quick select equipable gadgets unlocked is needed.");
	
	/* Validate the RDX header. */
	RacdoorFileHeader* header = buffer_get(rdx, 0, sizeof(RacdoorFileHeader), "RDX file header");
	CHECK(header->magic == FOURCC("RDX!"),
		"RDX file has bad magic number.");
	CHECK(header->version == RDX_FORMAT_VERSION,
		"RDX file has wrong version number (is %u, should be %u).",
		header->version, RDX_FORMAT_VERSION);
	
	Buffer payload = sub_buffer(rdx, header->payload_ofs, header->payload_size, "RDX payload");
	
	/* Prepare and encrypt the payload image. */
	u32 block_size = 2048;
	u32 max_size = block_size * save->level_count;
	
	u8* data = checked_malloc(max_size);
	memset(data, 0, max_size);
	
	CHECK(payload.size <= max_size, "RDX payload too big!");
	memcpy(data, payload.data, payload.size);
	
	printf("entry: %x\n", header->entry);
	
	if (key == 0)
	{
		srand(time(NULL));
		key = rand();
	}
	
	printf("key: %x\n", key);
	
	xor_crypt((u32*) data, (u32*) (data + max_size), key);
	
	/* Gather together a bunch of variables that are needed to setup the initial
	   hook. This is all explained in the arm function itself. */
	ExploitParams params;
	
	SaveBlock* voice_on = lookup_block(&save->game, BLOCK_HelpVoiceOn);
	SaveBlock* text_on = lookup_block(&save->game, BLOCK_HelpTextOn);
	CHECK(voice_on->size == 1, "Incorrectly sized VoiceOn block.");
	CHECK(text_on->size == 1, "Incorrectly sized TextOn block.");
	params.help_voice_on = (char*) voice_on->data;
	params.help_text_on = (char*) text_on->data;
	
	SaveBlock* help_data_messages = lookup_block(&save->game, BLOCK_HelpDataMessages);
	params.help_data_messages = (HelpDatum*) help_data_messages->data;
	params.help_data_messages_count = help_data_messages->size / sizeof(HelpDatum);
	params.enabled_help_message = header->symbols.help_message;
	
	SaveBlock* help_data_gadgets = lookup_block(&save->game, BLOCK_HelpDataGadgets);
	params.help_data_gadgets = (HelpDatum*) help_data_gadgets->data;
	params.help_data_gadgets_count = help_data_gadgets->size / sizeof(HelpDatum);
	params.enabled_help_gadget = header->symbols.help_gadget;
	
	SaveBlock* help_data_misc = lookup_block(&save->game, BLOCK_HelpDataMisc);
	params.help_data_misc = (HelpDatum*) help_data_misc->data;
	params.help_data_misc_count = help_data_misc->size / sizeof(HelpDatum);
	
	SaveBlock* help_log_pos = lookup_block(&save->game, BLOCK_HelpLogPos);
	CHECK(help_log_pos->size == 4, "Incorrectly sized HelpLogPos block.");
	params.help_log_pos = (int*) help_log_pos->data;
	
	params.help_log = header->symbols.help_log;
	params.initial_hook = header->symbols.initial_hook;
	
	SaveBlock* trampoline_save = lookup_block(&save->game, header->symbols.trampoline_block);
	CHECK((u64) header->symbols.trampoline_offset + 8 <= trampoline_save->size, "Symbol _racdoor_target_offset is too big.");
	params.trampoline = (u32*) &trampoline_save->data[header->symbols.trampoline_offset];
	params.trampoline_target = header->symbols.decryptor + DECRYPTOR_ENTRY_OFFSET;
	
	/* Setup the initial hook. */
	arm(&params);
	
	/* Generate some assembly code to decrypt the payload. */
	u8 decryptor_asm[256];
	gen_xor_decryptor((u32*) decryptor_asm, header->symbols.payload, header->symbols.payload_end, header->entry, key);
	
	/* Pack the decryptor into the save file. */
	u64 decryptor_asm_offset = 0;
	for (u32 i = 0; i < save->level_count; i++)
	{
		SaveBlock* dest = lookup_block(&save->levels[i], header->symbols.decryptor_block);
		u32 copy_size = MIN(dest->size, sizeof(decryptor_asm) - decryptor_asm_offset);
		memcpy(dest->data, decryptor_asm + decryptor_asm_offset, copy_size);
		
		decryptor_asm_offset += copy_size;
	}
	
	/* Write the encrypted payload image into the save file. */
	for (u32 i = 0; i < save->level_count; i++)
	{
		SaveBlock* dest = lookup_block(&save->levels[i], BLOCK_MapMask);
		memcpy(dest->data, data + i * block_size, block_size);
		
		u32 offset = (u32) (dest->data - (char*) save->header);
		printf("Written payload block for level %d at 0x%x.\n", i, offset);
	}
}
