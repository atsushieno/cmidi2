#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "cmidi2.h"


cmidi2_ump __ump;
cmidi2_ump* asUMP(int32_t i) { __ump = i; return &__ump; }

void testType0Messages()
{
    /* type 0 */
    assert(cmidi2_ump_noop(0) == 0);
    assert(cmidi2_ump_noop(1) == 0x01000000);
    assert(cmidi2_ump_jr_clock(0, 0) == 0x00100000);
    assert(cmidi2_ump_jr_clock(0, 1.0) == 0x00107A12);
    assert(cmidi2_ump_jr_timestamp(0, 0) == 0x00200000);
    assert(cmidi2_ump_jr_timestamp(1, 1.0) == 0x01207A12);

    assert(cmidi2_ump_get_jr_clock_time(asUMP(0x00107A12)) == 31250);
    assert(cmidi2_ump_get_jr_timestamp_timestamp(asUMP(0x01207A12)) == 31250);
}

void testType1Messages()
{
    assert(cmidi2_ump_system_message(1, 0xF1, 99, 0) == 0x11F16300);
    assert(cmidi2_ump_system_message(1, 0xF2, 99, 89) == 0x11F26359);
    assert(cmidi2_ump_system_message(1, 0xFF, 0, 0) == 0x11FF0000);

    assert(cmidi2_ump_get_system_message_byte2(asUMP(0x11F26359)) == 99);
    assert(cmidi2_ump_get_system_message_byte3(asUMP(0x11F26359)) == 89);
}

void testType2Messages()
{
    assert(cmidi2_ump_midi1_message(1, 0x80, 2, 65, 10) == 0x2182410A);
    assert(cmidi2_ump_midi1_note_off(1, 2, 65, 10) == 0x2182410A);
    assert(cmidi2_ump_midi1_note_on(1, 2, 65, 10) == 0x2192410A);
    assert(cmidi2_ump_midi1_paf(1, 2, 65, 10) == 0x21A2410A);
    assert(cmidi2_ump_midi1_cc(1, 2, 65, 10) == 0x21B2410A);
    assert(cmidi2_ump_midi1_program(1, 2, 29) == 0x21C21D00);
    assert(cmidi2_ump_midi1_caf(1, 2, 10) == 0x21D20A00);
    assert(cmidi2_ump_midi1_pitch_bend_direct(1, 2, 0) == 0x21E20000);
    assert(cmidi2_ump_midi1_pitch_bend_direct(1, 2, 1) == 0x21E20100);
    assert(cmidi2_ump_midi1_pitch_bend_direct(1, 2, 0x3FFF) == 0x21E27F7F);
    assert(cmidi2_ump_midi1_pitch_bend(1, 2, 0) == 0x21E20040);
    assert(cmidi2_ump_midi1_pitch_bend(1, 2, -8192) == 0x21E20000);
    assert(cmidi2_ump_midi1_pitch_bend(1, 2, 8191) == 0x21E27F7F);

    assert(cmidi2_ump_get_midi1_note_note(asUMP(0x2182410A)) == 65);
    assert(cmidi2_ump_get_midi1_note_note(asUMP(0x2192410A)) == 65);
    assert(cmidi2_ump_get_midi1_paf_note(asUMP(0x21A2410A)) == 65);
    assert(cmidi2_ump_get_midi1_cc_index(asUMP(0x21B2410A)) == 65);
    assert(cmidi2_ump_get_midi1_cc_data(asUMP(0x21B2410A)) == 10);
    assert(cmidi2_ump_get_midi1_program_program(asUMP(0x21C21D00)) == 29);
    assert(cmidi2_ump_get_midi1_caf_data(asUMP(0x21D20A00)) == 10);
    assert(cmidi2_ump_get_midi1_pitch_bend_data(asUMP(0x21E27F7F)) == 0x3FFF);
}

void testType3Messages()
{
    uint8_t gsReset[] = {0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7};
    uint8_t sysex12[] = {0xF0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0xF7}; // 12 bytes without 0xF0
    uint8_t sysex13[] = {0xF0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0xF7}; // 13 bytes without 0xF0

    assert(cmidi2_ump_sysex7_get_sysex_length(gsReset + 1) == 9);
    assert(cmidi2_ump_sysex7_get_sysex_length(gsReset) == 9); // skip 0xF0
    assert(cmidi2_ump_sysex7_get_sysex_length(sysex12 + 1) == 12);
    assert(cmidi2_ump_sysex7_get_sysex_length(sysex12) == 12); // skip 0xF0
    assert(cmidi2_ump_sysex7_get_sysex_length(sysex13 + 1) == 13);
    assert(cmidi2_ump_sysex7_get_sysex_length(sysex13) == 13); // skip 0xF0

    assert(cmidi2_ump_sysex7_get_num_packets(0) == 1);
    assert(cmidi2_ump_sysex7_get_num_packets(1) == 1);
    assert(cmidi2_ump_sysex7_get_num_packets(7) == 2);
    assert(cmidi2_ump_sysex7_get_num_packets(12) == 2);
    assert(cmidi2_ump_sysex7_get_num_packets(13) == 3);

    uint64_t v = cmidi2_ump_sysex7_direct(1, 0, 6, 0x41, 0x10, 0x42, 0x40, 0x00, 0x7F);
    assert(v == 0x310641104240007F);

    int length = cmidi2_ump_sysex7_get_sysex_length(gsReset);
    v = cmidi2_ump_sysex7_get_packet_of(1, length, gsReset, 0);
    assert(v == 0x3116411042124000); // skip F0 correctly.
    v = cmidi2_ump_sysex7_get_packet_of(1, length, gsReset, 1);
    assert(v == 0x31337F0041000000);

    length = cmidi2_ump_sysex7_get_sysex_length(sysex13);
    v = cmidi2_ump_sysex7_get_packet_of(1, length, sysex13, 0);
    assert(v == 0x3116000102030405); // status 1
    v = cmidi2_ump_sysex7_get_packet_of(1, length, sysex13, 1);
    assert(v == 0x3126060708090A0B); // status 2
    v = cmidi2_ump_sysex7_get_packet_of(1, length, sysex13, 2);
    assert(v == 0x31310C0000000000); // status 3
}

void testType4Messages()
{
    uint16_t pitch = cmidi2_ump_pitch_7_9_split(0x20, 0.5);
    assert(pitch == 0x4100);
    pitch = cmidi2_ump_pitch_7_9(32.5);
    assert(pitch == 0x4100);

    uint64_t v = cmidi2_ump_midi2_channel_message_8_8_16_16(1, CMIDI2_STATUS_NOTE_OFF, 2, 0x20, CMIDI2_ATTRIBUTE_TYPE_PITCH7_9, 0xFEDC, pitch);
    assert(v == 0x41822003FEDC4100);
    v = cmidi2_ump_midi2_channel_message_8_8_32(1, CMIDI2_STATUS_NOTE_OFF, 2, 0x20, CMIDI2_ATTRIBUTE_TYPE_PITCH7_9, 0x12345678);
    assert(v == 0x4182200312345678);

    v = cmidi2_ump_midi2_note_off(1, 2, 64, 0, 0x1234, 0);
    assert(v == 0x4182400012340000);
    v = cmidi2_ump_midi2_note_off(1, 2, 64, 3, 0x1234, pitch);
    assert(v == 0x4182400312344100);

    v = cmidi2_ump_midi2_note_on(1, 2, 64, 0, 0xFEDC, 0);
    assert(v == 0x41924000FEDC0000);
    v = cmidi2_ump_midi2_note_on(1, 2, 64, 3, 0xFEDC, pitch);
    assert(v == 0x41924003FEDC4100);

    v = cmidi2_ump_midi2_paf(1, 2, 64, 0x87654321);
    assert(v == 0x41A2400087654321);

    v = cmidi2_ump_midi2_cc(1, 2, 1, 0x87654321);
    assert(v == 0x41B2010087654321);

    v = cmidi2_ump_midi2_program(1, 2, 1, 29, 8, 1);
    assert(v == 0x41C200011D000801);

    v = cmidi2_ump_midi2_caf(1, 2, 0x87654321);
    assert(v == 0x41D2000087654321);

    v = cmidi2_ump_midi2_pitch_bend_direct(1, 2, 0x87654321);
    assert(v == 0x41E2000087654321);

    v = cmidi2_ump_midi2_pitch_bend(1, 2, 1);
    assert(v == 0x41E2000080000001);

    v = cmidi2_ump_midi2_per_note_rcc(1, 2, 56, 0x10, 0x33333333);
    assert(v == 0x4102381033333333);

    v = cmidi2_ump_midi2_per_note_acc(1, 2, 56, 0x10, 0x33333333);
    assert(v == 0x4112381033333333);

    v = cmidi2_ump_midi2_rpn(1, 2, 0x10, 0x20, 0x12345678);
    assert(v == 0x4122102012345678);

    v = cmidi2_ump_midi2_nrpn(1, 2, 0x10, 0x20, 0x12345678);
    assert(v == 0x4132102012345678);

    v = cmidi2_ump_midi2_relative_rpn(1, 2, 0x10, 0x20, 0x12345678);
    assert(v == 0x4142102012345678);

    v = cmidi2_ump_midi2_relative_nrpn(1, 2, 0x10, 0x20, 0x12345678);
    assert(v == 0x4152102012345678);

    v = cmidi2_ump_midi2_per_note_pitch_bend_direct(1, 2, 56, 0x87654321);
    assert(v == 0x4162380087654321);

    v = cmidi2_ump_midi2_per_note_pitch_bend(1, 2, 56, 1);
    assert(v == 0x4162380080000001);

    v = cmidi2_ump_midi2_per_note_management(1, 2, 56, CMIDI2_PER_NOTE_MANAGEMENT_DETACH);
    assert(v == 0x41F2380200000000);
}

void testType5Messages_sysex()
{
    uint8_t gsReset[] = {0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41};
    uint8_t sysex27[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

    assert(cmidi2_ump_sysex8_get_num_packets(0) == 1);
    assert(cmidi2_ump_sysex8_get_num_packets(1) == 1);
    assert(cmidi2_ump_sysex8_get_num_packets(13) == 1);
    assert(cmidi2_ump_sysex8_get_num_packets(14) == 2);
    assert(cmidi2_ump_sysex8_get_num_packets(26) == 2);
    assert(cmidi2_ump_sysex8_get_num_packets(27) == 3);

    uint64_t result1, result2;

    int length = 9;
    cmidi2_ump_sysex8_get_packet_of(1, 7, length, gsReset, 0, &result1, &result2);
    assert(result1 == 0x510A074110421240);
    assert(result2 == 0x007F004100000000);

    length = 27;
    cmidi2_ump_sysex8_get_packet_of(1, 7, length, sysex27, 0, &result1, &result2);
    assert(result1 == 0x511E070102030405);
    assert(result2 == 0x060708090A0B0C0D);
    cmidi2_ump_sysex8_get_packet_of(1, 7, length, sysex27, 1, &result1, &result2);
    assert(result1 == 0x512E070E0F101112);
    assert(result2 == 0x131415161718191A);
    cmidi2_ump_sysex8_get_packet_of(1, 7, length, sysex27, 2, &result1, &result2);
    assert(result1 == 0x5132071B00000000);
    assert(result2 == 0x0000000000000000);
}

void testType5Messages_mds()
{
    int length = cmidi2_ump_mds_get_num_payloads(0);
    assert(length == 0);
    length = cmidi2_ump_mds_get_num_payloads(1);
    assert(length == 1);
    length = cmidi2_ump_mds_get_num_payloads(14);
    assert(length == 1);
    length = cmidi2_ump_mds_get_num_payloads(15);
    assert(length == 2);
    length = cmidi2_ump_mds_get_num_payloads(65536);
    assert(length == 4682);
    length = cmidi2_ump_mds_get_num_payloads(65535 * 14);
    assert(length == 65535);
    length = cmidi2_ump_mds_get_num_payloads(65535 * 14 + 1);
    assert(length < 0);

    length = cmidi2_ump_mds_get_num_chunks(0);
    assert(length == 0);
    length = cmidi2_ump_mds_get_num_chunks(1);
    assert(length == 1);
    length = cmidi2_ump_mds_get_num_chunks(14 * 65536);
    assert(length == 1);
    length = cmidi2_ump_mds_get_num_chunks(14 * 65536 + 1);
    assert(length == 2);
    length = cmidi2_ump_mds_get_num_chunks(4294967295);
    assert(length == 4682);

    uint64_t result1, result2;

    cmidi2_ump_mds_get_header(1, 2, 100, 4, 3, 100, 101, 102, 103, &result1, &result2);
    assert(result1 == 0x5182640004000300);
    assert(result2 == 0x6400650066006700);

    uint8_t srcData[140];
    memset(srcData, 0, sizeof(srcData));
    for (int i = 0; i < 100; i++)
        srcData[i] = i;

    cmidi2_ump_mds_get_payload_of(1, 2, 100, srcData, &result1, &result2);
    assert(result1 == 0x5192000102030405);
    assert(result2 == 0x060708090A0B0C0D);
    cmidi2_ump_mds_get_payload_of(1, 2, 100, srcData + 14 * 7, &result1, &result2);
    assert(result1 == 0x5192626300000000);
    assert(result2 == 0);
}

void testType5Messages()
{
    testType5Messages_sysex();
    testType5Messages_mds();
}

void testForEach()
{
    uint64_t ump[] = {
	cmidi2_ump_jr_clock(1, 0.0) * 0x100000000 + cmidi2_ump_jr_timestamp(1, 0),
	cmidi2_ump_midi2_note_on(1, 1, 60, 0, 120 * 0x100, 0),
	cmidi2_ump_jr_clock(1, 0.0) * 0x100000000 + cmidi2_ump_jr_timestamp(1, 1),
	cmidi2_ump_midi2_note_off(1, 1, 60, 0, 120 * 0x100, 0),
    };

    uint8_t buf[sizeof(ump)];
    for (int i = 0; i < sizeof(buf) / sizeof(uint64_t); i++) {
        for (int b = 0; b < 8; b++) {
            uint8_t v = (ump[i] >> (7 - b) * 8) & 0xFF;
            buf[i * 8 + b] = v;
        }
    }

    uint64_t expected [] = {
        0x01100000,
        0x01200000,
        0x41913c0078000000,
        0x01100000,
        0x01207a12,
        0x41813c0078000000
    };

    int current = 0;

    CMIDI2_UMP_SEQUENCE_FOREACH((void*) buf, sizeof(buf), iter) {
        uint64_t ret;
        switch (cmidi2_ump_get_num_bytes(cmidi2_ump_read_uint32_bytes(iter))) {
        case 4:
            ret = cmidi2_ump_read_uint32_bytes(iter);
            printf("32bit DATA: %08"PRIX32" \n", (uint32_t) ret);
            assert(ret == expected[current++]);
            break;
        case 8:
            ret = cmidi2_ump_read_uint64_bytes(iter);
            printf("64bit DATA: %016"PRIX64" \n", ret);
            assert(ret == expected[current++]);
            break;
        default:
            printf("unexpected bytes at %d\n", current);
            assert(false);
            break;
        }
    }
}

int main ()
{
    testType0Messages();
    testType1Messages();
    testType2Messages();
    testType3Messages();
    testType4Messages();
    testType5Messages();
    testForEach();
    uint8_t bytes [] = {0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3};
    printf("%d\n", cmidi2_ump_get_channel((cmidi2_ump*) bytes));
    return 0;
}
