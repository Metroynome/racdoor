#include <racdoor/save.h>
#include <racdoor/exploit.h>
#include <string.h>

int main(int argc, char** argv)
{
	CHECK(argc == 4, "usage: install-probe input-save probe.bin output-save");
	CHECK(strcmp(argv[1], argv[3]) != 0, "Use a separate disposable save.");
	Buffer file = read_file(argv[1]);
	Buffer code = read_file(argv[2]);
	SaveSlot save = parse_save(file);
	CHECK(save.game.block_count == UYA_GAME_BLOCK_COUNT && save.level_count == 37,
		"Expected UYA save layout.");
	SaveBlock* level = lookup_block(&save.game, BLOCK_Level);
	CHECK(level->size == 4 && *(u32*)level->data == 3, "Probe requires Phoenix save.");
	SaveBlock* queue = lookup_block(&save.game, 39);
	SaveBlock* map = lookup_block(&save.levels[0], BLOCK_MapMask);
	SaveBlock* position = lookup_block(&save.game, BLOCK_HelpLogPos);
	SaveBlock* flags = lookup_block(&save.game, 5);
	CHECK(queue->size == 0x80 && map->size == 2048 && position->size == 4
		&& flags->size == 0x110 && code.size <= 0x400, "Unexpected probe layout.");
	s32 count;
	memcpy(&count, position->data, 4);
	CHECK(count >= 0 && count < 400, "Expected ordinary help history.");
	memcpy(map->data, code.data, code.size);
	memcpy(map->data + 0x400, queue->data + 0x0c, 12);
	memcpy(map->data + 0x40c, &count, 4);
	memset(map->data + 0x420, 0, 4);
	/* Both camera variants land on this nop/jump/nop sequence. */
	u32 trampoline[] = {0, 0x0806a4c6, 0};
	memcpy(queue->data + 0x0c, trampoline, sizeof(trampoline));
	for (u32 type = BLOCK_HelpDataMessages; type <= BLOCK_HelpDataMisc; type++) {
		SaveBlock* records = lookup_block(&save.game, type);
		CHECK(records->size % 12 == 0, "Unexpected help record size.");
		for (u32 i = 0; i < records->size; i += 12) {
			u16 disabled = 0xffff;
			memcpy(records->data + i, &disabled, 2);
		}
	}
	SaveBlock* messages = lookup_block(&save.game, BLOCK_HelpDataMessages);
	CHECK(messages->size == 356 * 12, "Unexpected message count.");
	memset(messages->data + 0x11e * 12, 0, 24);
	flags->data[0xdf] &= ~4;
	s32 index = helpLogIndex(0x001a8ec8, 0x0013d27c, 2);
	memcpy(position->data, &index, 4);
	update_checksums(&save);
	write_file(argv[3], file);
	/* Reparse to verify every section checksum before reporting success. */
	Buffer output = read_file(argv[3]);
	parse_save(output);
	printf("PAL diagnostic save prepared; no DNAS or multiplayer persistence.\n");
	return 0;
}
