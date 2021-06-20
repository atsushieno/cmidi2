#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>
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

int testUMP ()
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

// MIDI CI

#define CMIDI2_CI_PROTOCOL_NEGOTIATION_SUPPORTED 2
#define CMIDI2_CI_PROFILE_CONFIGURATION_SUPPORTED 4
#define CMIDI2_CI_PROPERTY_EXCHANGE_SUPPORTED 8

void testDiscoveryMessages()
{
    // Service Discovery (Inquiry)
    uint8_t expected1[] = {0x7E, 0x7F, 0x0D, 0x70, 1,
        0x10, 0x10, 0x10, 0x10, 0x7F, 0x7F, 0x7F, 0x7F,
        0x56, 0x34, 0x12, 0x57, 0x13, 0x68, 0x24,
        // LAMESPEC: Software Revision Level does not mention in which endianness this field is stored.
        0x7F, 0x5F, 0x3F, 0x1F,
        0b00001110,
        0x00, 0x02, 0, 0
        };
    uint8_t actual1[29];
    cmidi2_ci_put_discovery(actual1, 1, 0x10101010,
        0x123456, 0x1357, 0x2468, 0x1F3F5F7F,
        CMIDI2_CI_PROTOCOL_NEGOTIATION_SUPPORTED | CMIDI2_CI_PROFILE_CONFIGURATION_SUPPORTED | CMIDI2_CI_PROPERTY_EXCHANGE_SUPPORTED,
        512);
    //for (int i = 0; i < 29; i++) printf("%x ", actual1[i]); puts("");
    assert(memcmp(expected1, actual1, 29) == 0);

    // Service Discovery Reply
    uint8_t actual2[29];
    cmidi2_ci_put_discovery_reply(actual2, 1, 0x10101010, 0x20202020,
        0x123456, 0x1357, 0x2468, 0x1F3F5F7F,
        CMIDI2_CI_PROTOCOL_NEGOTIATION_SUPPORTED | CMIDI2_CI_PROFILE_CONFIGURATION_SUPPORTED | CMIDI2_CI_PROPERTY_EXCHANGE_SUPPORTED,
        512);
    assert(actual2[3] == 0x71);
    for (int i = 9; i < 13; i++) assert(actual2[i] == 0x20); // destination ID is not 7F7F7F7F.

    // Invalidate MUID
    uint8_t expected3[] = {0x7E, 0x7F, 0x0D, 0x7E, 1,
        0x10, 0x10, 0x10, 0x10, 0x7F, 0x7F, 0x7F, 0x7F, 0x20, 0x20, 0x20, 0x20};
    uint8_t actual3[17];
    cmidi2_ci_put_discovery_invalidate_muid(actual3, 1, 0x10101010, 0x20202020);
    //for (int i = 0; i < 17; i++) printf("%x ", actual3[i]); puts("");
    assert(memcmp(expected3, actual3, 17) == 0);

    // NAK
    uint8_t expected4[] = {0x7E, 5, 0x0D, 0x7F, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20};
    uint8_t actual4[13];
    cmidi2_ci_put_discovery_nak(actual4, 5, 1, 0x10101010, 0x20202020);
    //for (int i = 0; i < 13; i++) printf("%x ", actual4[i]); puts("");
    assert(memcmp(expected4, actual4, 13) == 0);
}

void testProtocolNegotiationMessages()
{
    cmidi2_ci_protocol_type_info infos[] = {
        {1, 0, 0x10, 0, 0},
        {2, 0, 0x20, 0, 0}
    };

    // Service Discovery (Inquiry)
    uint8_t expected1[] = {0x7E, 0x7F, 0x0D, 0x10, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1, 2,
        1, 0, 0x10, 0, 0,
        2, 0, 0x20, 0, 0};
    uint8_t actual1[25];
    cmidi2_ci_put_protocol_negotiation(actual1, false, 0x10101010, 0x20202020,
        1, 2, infos);
    //for (int i = 0; i < 25; i++) printf("%x ", actual1[i]); puts("");
    assert(memcmp(expected1, actual1, 25) == 0);

    // Service Discovery Reply
    uint8_t actual2[25];
    cmidi2_ci_put_protocol_negotiation(actual2, true, 0x10101010, 0x20202020,
        1, 2, infos);
    //for (int i = 0; i < 25; i++) printf("%x ", actual1[i]); puts("");
    assert(actual2[3] == 0x11);

    // Set New Protocol
    uint8_t expected3[] = {0x7E, 0x7F, 0x0D, 0x12, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1, 2, 0, 0x20, 0, 0};
    uint8_t actual3[19];
    cmidi2_ci_put_protocol_set(actual3, 0x10101010, 0x20202020,
        1, infos[1]);
    //for (int i = 0; i < 19; i++) printf("%x ", actual3[i]); puts("");
    assert(memcmp(expected3, actual3, 19) == 0);

    // Test New Protocol - Initiator to Recipient
    uint8_t testData[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t expected4[] = {0x7E, 0x7F, 0x0D, 0x13, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1};
    uint8_t actual4[14 + 48];
    cmidi2_ci_put_protocol_test(actual4, true, 0x10101010, 0x20202020,
        1, testData);
    //for (int i = 0; i < 14; i++) printf("%x ", actual4[i]); puts("");
    assert(memcmp(expected4, actual4, 14) == 0);
    assert(memcmp(testData, actual4 + 14, 48) == 0);

    // Test New Protocol - Responder to Initiator
    uint8_t actual5[14 + 48];
    cmidi2_ci_put_protocol_test(actual5, false, 0x10101010, 0x20202020,
        1, testData);
    assert(actual5[3] == 0x14);

    // Confirmation New Protocol Established
    uint8_t expected6[] = {0x7E, 0x7F, 0x0D, 0x15, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1};
    uint8_t actual6[14];
    cmidi2_ci_put_protocol_confirm_established(actual6, 0x10101010, 0x20202020, 1);
    //for (int i = 0; i < 19; i++) printf("%x ", actual6[i]); puts("");
    assert(memcmp(expected6, actual6, 14) == 0);
}

void testProfileConfigurationMessages()
{
    // Profile Inquiry
    uint8_t expected1[] = {0x7E, 5, 0x0D, 0x20, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20};
    uint8_t actual1[13];
    cmidi2_ci_put_profile_inquiry(actual1, 5, 0x10101010, 0x20202020);
    //for (int i = 0; i < 13; i++) printf("%x ", actual1[i]); puts("");
    assert(memcmp(expected1, actual1, 13) == 0);

    // Profile Inquiry Reply
    cmidi2_profile_id profiles1[] = { {1,2,3,4,5}, {6,7,8,9,10} };
    cmidi2_profile_id profiles2[] = { {11,12,13,14,15}, {16,17,18,19,20} };
    uint8_t expected2[] = {0x7E, 5, 0x0D, 0x21, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        2,
        1, 2, 3, 4, 5,
        6, 7, 8, 9, 10,
        2,
        11, 12, 13, 14, 15,
        16, 17, 18, 19, 20};
    uint8_t actual2[35];
    cmidi2_ci_put_profile_inquiry_reply(actual2, 5, 0x10101010, 0x20202020,
        2, profiles1, 2, profiles2);
    //for (int i = 0; i < 35; i++) printf("%x ", actual2[i]); puts("");
    assert(memcmp(expected2, actual2, 35) == 0);

    // Set Profile On
    uint8_t expected3[] = {0x7E, 5, 0x0D, 0x22, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1, 2, 3, 4, 5};
    uint8_t actual3[18];
    cmidi2_ci_put_profile_set(actual3, 5, true, 0x10101010, 0x20202020,
        profiles1[0]);
    //for (int i = 0; i < 18; i++) printf("%x ", actual1[i]); puts("");
    assert(memcmp(expected3, actual3, 18) == 0);

    // Set Profile Off
    uint8_t actual4[18];
    cmidi2_ci_put_profile_set(actual4, 5, false, 0x10101010, 0x20202020,
        profiles1[0]);
    assert(actual4[3] == 0x23);

    // Profile Enabled Report
    uint8_t expected5[] = {0x7E, 5, 0x0D, 0x24, 1,
        0x10, 0x10, 0x10, 0x10, 0x7F, 0x7F, 0x7F, 0x7F,
        1, 2, 3, 4, 5};
    uint8_t actual5[18];
    cmidi2_ci_put_profile_report(actual5, 5, true, 0x10101010,
        profiles1[0]);
    //for (int i = 0; i < 18; i++) printf("%x ", actual5[i]); puts("");
    assert(memcmp(expected5, actual5, 18) == 0);

    // Profile Disabled Report
    uint8_t actual6[18];
    cmidi2_ci_put_profile_report(actual6, 5, false, 0x10101010,
        profiles1[0]);
    assert(actual6[3] == 0x25);

    // Profile Specific Data
    uint8_t expected7[] = {0x7E, 5, 0x0D, 0x2F, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        1, 2, 3, 4, 5,
        8, 0, 0, 0,
        8,7,6,5,4,3,2,1};
    uint8_t actual7[30];
    uint8_t data[] = {8,7,6,5,4,3,2,1};
    cmidi2_ci_put_profile_specific_data(actual7, 5, 0x10101010, 0x20202020,
        profiles1[0], 8, data);
    //for (int i = 0; i < 30; i++) printf("%x ", actual7[i]); puts("");
    assert(memcmp(expected7, actual7, 30) == 0);
}

void testPropertyExchangeMessages()
{
    uint8_t header[] = {11,22,33,44};
    uint8_t data[] = {55,66,77,88,99};

    // Property Inquiry
    uint8_t expected1[] = {0x7E, 5, 0x0D, 0x30, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        16};
    uint8_t actual1[14];
    cmidi2_ci_put_property_get_capabilities(actual1, 5, false, 0x10101010, 0x20202020, 16);
    //for (int i = 0; i < 14; i++) printf("%x ", actual1[i]); puts("");
    assert(memcmp(expected1, actual1, 14) == 0);

    // Property Inquiry Reply
    uint8_t actual2[14];
    cmidi2_ci_put_property_get_capabilities(actual2, 5, true, 0x10101010, 0x20202020, 16);
    assert(actual2[3] == 0x31);

    // Has Property Data
    uint8_t expected3[] = {0x7E, 5, 0x0D, 0x32, 1,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
        2,
        4, 0,
        11,22,33,44,
        3, 0,
        1, 0,
        5, 0,
        55,66,77,88,99};
    uint8_t actual3[31];
    cmidi2_ci_put_property_common(actual3, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_HAS_DATA_INQUIRY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    //for (int i = 0; i < 31; i++) printf("%x ", actual3[i]); puts("");
    assert(memcmp(expected3, actual3, 31) == 0);

    // Reply to Has Property Data
    uint8_t actual4[31];
    cmidi2_ci_put_property_common(actual4, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_HAS_DATA_REPLY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual4[3] == 0x33);

    // Get Property Data
    uint8_t actual5[31];
    cmidi2_ci_put_property_common(actual5, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA_INQUIRY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual5[3] == 0x34);

    // Reply to Get Property Data
    uint8_t actual6[31];
    cmidi2_ci_put_property_common(actual6, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_GET_DATA_REPLY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual6[3] == 0x35);

    // Set Property Data
    uint8_t actual7[31];
    cmidi2_ci_put_property_common(actual7, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA_INQUIRY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual7[3] == 0x36);

    // Reply to Set Property Data
    uint8_t actual8[31];
    cmidi2_ci_put_property_common(actual8, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_SET_DATA_REPLY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual8[3] == 0x37);

    // Subscription
    uint8_t actual9[31];
    cmidi2_ci_put_property_common(actual9, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual9[3] == 0x38);

    // Reply to Subscription
    uint8_t actual10[31];
    cmidi2_ci_put_property_common(actual10, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_SUBSCRIBE_REPLY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual10[3] == 0x39);

    // Notify
    uint8_t actual11[31];
    cmidi2_ci_put_property_common(actual11, 5, CMIDI2_CI_SUB_ID_2_PROPERTY_NOTIFY,
        0x10101010, 0x20202020,
        2, 4, header, 3, 1, 5, data);
    assert(actual11[3] == 0x3F);
}

int testMidiCI ()
{
    testDiscoveryMessages();
    testProtocolNegotiationMessages();
    testProfileConfigurationMessages();
    testPropertyExchangeMessages();
    return 0;
}

// main

int main ()
{
    testUMP();
    testMidiCI();
    return 0;
}