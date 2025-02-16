#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///=============================================================================
/// DECIMAL LITERALS
TEST_CASE(Integer, Dec, 0, "1 2 3", {1, 2, 3})
TEST_CASE(Integer, Dec, 1, "111 222 333", {111, 222, 333})
TEST_CASE(Integer, Dec, 2, "111111 222222 333333", {111111, 222222, 333333})
TEST_CASE(Integer, Dec, 3, "340282366920938463463374607431768211455",
          {Token(IntL, "340282366920938463463374607431768211455")})
TEST_CASE(Integer, Dec, 4, "340282366920938463463374607431768211456", {});
TEST_CASE(Integer, Dec, 5, "123i32", {123, Token(Name, "i32")})
TEST_CASE(Integer, Dec, 6, "0", {0})
TEST_CASE(Integer, Dec, 7, "79_7c64_919", {})
TEST_CASE(Integer, Dec, 8, "1595_298_38", {159529838})
TEST_CASE(Integer, Dec, 9, "2225_046_65", {222504665})
TEST_CASE(Integer, Dec, 10, "3190_596_76", {319059676})
TEST_CASE(Integer, Dec, 11, "3988_140_59", {398814059})
TEST_CASE(Integer, Dec, 12, "4450_093_30", {445009330})
TEST_CASE(Integer, Dec, 13, "5079_900_21", {507990021})
TEST_CASE(Integer, Dec, 14, "6381_193_52", {638119352})
TEST_CASE(Integer, Dec, 15, "5836_595_35", {583659535})
TEST_CASE(Integer, Dec, 16, "7976_281_18", {797628118})
TEST_CASE(Integer, Dec, 17, "7263_875_53", {726387553})
TEST_CASE(Integer, Dec, 18, "8900_186_60", {890018660})
TEST_CASE(Integer, Dec, 19, "8355_529_79", {835552979})
TEST_CASE(Integer, Dec, 20, "101598_004_2", {1015980042})
TEST_CASE(Integer, Dec, 21, "9447_500_13", {944750013})
TEST_CASE(Integer, Dec, 22, "127623_870_4", {1276238704})
TEST_CASE(Integer, Dec, 23, "122164_192_7", {1221641927})
TEST_CASE(Integer, Dec, 24, "116731_907_0", {1167319070})
TEST_CASE(Integer, Dec, 25, "109595_792_9", {1095957929})
TEST_CASE(Integer, Dec, 26, "159525_623_6", {1595256236})
TEST_CASE(Integer, Dec, 27, "154066_537_1", {1540665371})
TEST_CASE(Integer, Dec, 28, "145277_510_6", {1452775106})
TEST_CASE(Integer, Dec, 29, "138140_350_9", {1381403509})
TEST_CASE(Integer, Dec, 30, "178003_732_0", {1780037320})
TEST_CASE(Integer, Dec, 31, "185966_067_1", {1859660671})
TEST_CASE(Integer, Dec, 32, "167110_595_8", {1671105958})
TEST_CASE(Integer, Dec, 33, "173395_560_1", {1733955601})
TEST_CASE(Integer, Dec, 34, "203196_008_4", {2031960084})
TEST_CASE(Integer, Dec, 35, "211159_389_1", {2111593891})
TEST_CASE(Integer, Dec, 36, "188950_002_6", {1889500026})
TEST_CASE(Integer, Dec, 37, "195234_375_7", {1952343757})
TEST_CASE(Integer, Dec, 38, "255247_740_8", {2552477408})
TEST_CASE(Integer, Dec, 39, "263210_069_5", {2632100695})
TEST_CASE(Integer, Dec, 40, "244328_385_4", {2443283854})
TEST_CASE(Integer, Dec, 41, "250613_356_1", {2506133561})
TEST_CASE(Integer, Dec, 42, "233463_814_0", {2334638140})
TEST_CASE(Integer, Dec, 43, "241427_188_3", {2414271883})
TEST_CASE(Integer, Dec, 44, "219191_585_8", {2191915858})
TEST_CASE(Integer, Dec, 45, "225475_965_3", {2254759653})
TEST_CASE(Integer, Dec, 46, "319051_247_2", {3190512472})
TEST_CASE(Integer, Dec, 47, "313591_575_9", {3135915759})
TEST_CASE(Integer, Dec, 48, "308133_074_2", {3081330742})
TEST_CASE(Integer, Dec, 49, "300996_953_7", {3009969537})
TEST_CASE(Integer, Dec, 50, "290555_021_2", {2905550212})
TEST_CASE(Integer, Dec, 51, "285095_941_1", {2850959411})
TEST_CASE(Integer, Dec, 52, "276280_701_8", {2762807018})
TEST_CASE(Integer, Dec, 53, "269143_535_7", {2691435357})
TEST_CASE(Integer, Dec, 54, "356007_464_0", {3560074640})
TEST_CASE(Integer, Dec, 55, "350561_488_7", {3505614887})
TEST_CASE(Integer, Dec, 56, "371932_134_2", {3719321342})
TEST_CASE(Integer, Dec, 57, "364808_071_3", {3648080713})
TEST_CASE(Integer, Dec, 58, "334221_191_6", {3342211916})
TEST_CASE(Integer, Dec, 59, "328774_629_9", {3287746299})
TEST_CASE(Integer, Dec, 60, "346791_120_2", {3467911202})
TEST_CASE(Integer, Dec, 61, "339668_110_9", {3396681109})
TEST_CASE(Integer, Dec, 62, "406392_016_8", {4063920168})
TEST_CASE(Integer, Dec, 63, "414368_502_3", {4143685023})
TEST_CASE(Integer, Dec, 64, "422318_778_2", {4223187782})
TEST_CASE(Integer, Dec, 65, "428616_267_3", {4286162673})
TEST_CASE(Integer, Dec, 66, "377900_005_2", {3779000052})
TEST_CASE(Integer, Dec, 67, "385875_437_1", {3858754371})
TEST_CASE(Integer, Dec, 68, "390468_751_4", {3904687514})
TEST_CASE(Integer, Dec, 69, "396766_826_9", {3967668269})
TEST_CASE(Integer, Dec, 70, "8812_258_47", {881225847})
TEST_CASE(Integer, Dec, 71, "8099_875_20", {809987520})
TEST_CASE(Integer, Dec, 72, "102369_154_5", {1023691545})
TEST_CASE(Integer, Dec, 73, "9692_340_94", {969234094})
TEST_CASE(Integer, Dec, 74, "6628_328_11", {662832811})
TEST_CASE(Integer, Dec, 75, "5916_004_12", {591600412})
TEST_CASE(Integer, Dec, 76, "7717_677_49", {771767749})
TEST_CASE(Integer, Dec, 77, "7172_998_26", {717299826})
TEST_CASE(Integer, Dec, 78, "3113_363_99", {311336399})
TEST_CASE(Integer, Dec, 79, "3743_089_84", {374308984})
TEST_CASE(Integer, Dec, 80, "4538_139_21", {453813921})
TEST_CASE(Integer, Dec, 81, "5335_764_70", {533576470})
TEST_CASE(Integer, Dec, 82, "25_881_363", {25881363})
TEST_CASE(Integer, Dec, 83, "88_864_420", {88864420})
TEST_CASE(Integer, Dec, 84, "1347_953_89", {134795389})
TEST_CASE(Integer, Dec, 85, "2145_520_10", {214552010})
TEST_CASE(Integer, Dec, 86, "202320_563_9", {2023205639})
TEST_CASE(Integer, Dec, 87, "208605_764_8", {2086057648})
TEST_CASE(Integer, Dec, 88, "189723_863_3", {1897238633})
TEST_CASE(Integer, Dec, 89, "197686_422_2", {1976864222})
TEST_CASE(Integer, Dec, 90, "180485_269_9", {1804852699})
TEST_CASE(Integer, Dec, 91, "186769_418_8", {1867694188})
TEST_CASE(Integer, Dec, 92, "164534_034_1", {1645340341})
TEST_CASE(Integer, Dec, 93, "172497_177_8", {1724971778})
TEST_CASE(Integer, Dec, 94, "158749_663_9", {1587496639})
TEST_CASE(Integer, Dec, 95, "151613_312_8", {1516133128})
TEST_CASE(Integer, Dec, 96, "146155_054_5", {1461550545})
TEST_CASE(Integer, Dec, 97, "140695_152_6", {1406951526})
TEST_CASE(Integer, Dec, 98, "130201_609_9", {1302016099})
TEST_CASE(Integer, Dec, 99, "123064_674_0", {1230646740})
TEST_CASE(Integer, Dec, 100, "114249_191_7", {1142491917})

///=============================================================================
/// EXPLICIT DECIMAL LITERALS
TEST_CASE(Integer, ExpDec, 0, "0d1 0d2 0d3", {1, 2, 3})
TEST_CASE(Integer, ExpDec, 1, "0d111 0d222 0d333", {111, 222, 333})
TEST_CASE(Integer, ExpDec, 2, "0d111111 0d222222 0d333333", {111111, 222222, 333333})
TEST_CASE(Integer, ExpDec, 3, "0d340282366920938463463374607431768211455",
          {Token(IntL, "340282366920938463463374607431768211455")})
TEST_CASE(Integer, ExpDec, 4, "0d340282366920938463463374607431768211456", {});
TEST_CASE(Integer, ExpDec, 5, "0d123i32", {123, Token(Name, "i32")})
TEST_CASE(Integer, ExpDec, 6, "0d0", {0})
TEST_CASE(Integer, ExpDec, 7, "0d79_7c64_919", {})
TEST_CASE(Integer, ExpDec, 8, "0d1595_298_38", {159529838})
TEST_CASE(Integer, ExpDec, 9, "0d2225_046_65", {222504665})
TEST_CASE(Integer, ExpDec, 10, "0d3190_596_76", {319059676})
TEST_CASE(Integer, ExpDec, 11, "0d3988_140_59", {398814059})
TEST_CASE(Integer, ExpDec, 12, "0d4450_093_30", {445009330})
TEST_CASE(Integer, ExpDec, 13, "0d5079_900_21", {507990021})
TEST_CASE(Integer, ExpDec, 14, "0d6381_193_52", {638119352})
TEST_CASE(Integer, ExpDec, 15, "0d5836_595_35", {583659535})
TEST_CASE(Integer, ExpDec, 16, "0d7976_281_18", {797628118})
TEST_CASE(Integer, ExpDec, 17, "0d7263_875_53", {726387553})
TEST_CASE(Integer, ExpDec, 18, "0d8900_186_60", {890018660})
TEST_CASE(Integer, ExpDec, 19, "0d8355_529_79", {835552979})
TEST_CASE(Integer, ExpDec, 20, "0d101598_004_2", {1015980042})
TEST_CASE(Integer, ExpDec, 21, "0d9447_500_13", {944750013})
TEST_CASE(Integer, ExpDec, 22, "0d127623_870_4", {1276238704})
TEST_CASE(Integer, ExpDec, 23, "0d122164_192_7", {1221641927})
TEST_CASE(Integer, ExpDec, 24, "0d116731_907_0", {1167319070})
TEST_CASE(Integer, ExpDec, 25, "0d109595_792_9", {1095957929})
TEST_CASE(Integer, ExpDec, 26, "0d159525_623_6", {1595256236})
TEST_CASE(Integer, ExpDec, 27, "0d154066_537_1", {1540665371})
TEST_CASE(Integer, ExpDec, 28, "0d145277_510_6", {1452775106})
TEST_CASE(Integer, ExpDec, 29, "0d138140_350_9", {1381403509})
TEST_CASE(Integer, ExpDec, 30, "0d178003_732_0", {1780037320})
TEST_CASE(Integer, ExpDec, 31, "0d185966_067_1", {1859660671})
TEST_CASE(Integer, ExpDec, 32, "0d167110_595_8", {1671105958})
TEST_CASE(Integer, ExpDec, 33, "0d173395_560_1", {1733955601})
TEST_CASE(Integer, ExpDec, 34, "0d203196_008_4", {2031960084})
TEST_CASE(Integer, ExpDec, 35, "0d211159_389_1", {2111593891})
TEST_CASE(Integer, ExpDec, 36, "0d188950_002_6", {1889500026})
TEST_CASE(Integer, ExpDec, 37, "0d195234_375_7", {1952343757})
TEST_CASE(Integer, ExpDec, 38, "0d255247_740_8", {2552477408})
TEST_CASE(Integer, ExpDec, 39, "0d263210_069_5", {2632100695})
TEST_CASE(Integer, ExpDec, 40, "0d244328_385_4", {2443283854})
TEST_CASE(Integer, ExpDec, 41, "0d250613_356_1", {2506133561})
TEST_CASE(Integer, ExpDec, 42, "0d233463_814_0", {2334638140})
TEST_CASE(Integer, ExpDec, 43, "0d241427_188_3", {2414271883})
TEST_CASE(Integer, ExpDec, 44, "0d219191_585_8", {2191915858})
TEST_CASE(Integer, ExpDec, 45, "0d225475_965_3", {2254759653})
TEST_CASE(Integer, ExpDec, 46, "0d319051_247_2", {3190512472})
TEST_CASE(Integer, ExpDec, 47, "0d313591_575_9", {3135915759})
TEST_CASE(Integer, ExpDec, 48, "0d308133_074_2", {3081330742})
TEST_CASE(Integer, ExpDec, 49, "0d300996_953_7", {3009969537})
TEST_CASE(Integer, ExpDec, 50, "0d290555_021_2", {2905550212})
TEST_CASE(Integer, ExpDec, 51, "0d285095_941_1", {2850959411})
TEST_CASE(Integer, ExpDec, 52, "0d276280_701_8", {2762807018})
TEST_CASE(Integer, ExpDec, 53, "0d269143_535_7", {2691435357})
TEST_CASE(Integer, ExpDec, 54, "0d356007_464_0", {3560074640})
TEST_CASE(Integer, ExpDec, 55, "0d350561_488_7", {3505614887})
TEST_CASE(Integer, ExpDec, 56, "0d371932_134_2", {3719321342})
TEST_CASE(Integer, ExpDec, 57, "0d364808_071_3", {3648080713})
TEST_CASE(Integer, ExpDec, 58, "0d334221_191_6", {3342211916})
TEST_CASE(Integer, ExpDec, 59, "0d328774_629_9", {3287746299})
TEST_CASE(Integer, ExpDec, 60, "0d346791_120_2", {3467911202})
TEST_CASE(Integer, ExpDec, 61, "0d339668_110_9", {3396681109})
TEST_CASE(Integer, ExpDec, 62, "0d406392_016_8", {4063920168})
TEST_CASE(Integer, ExpDec, 63, "0d414368_502_3", {4143685023})
TEST_CASE(Integer, ExpDec, 64, "0d422318_778_2", {4223187782})
TEST_CASE(Integer, ExpDec, 65, "0d428616_267_3", {4286162673})
TEST_CASE(Integer, ExpDec, 66, "0d377900_005_2", {3779000052})
TEST_CASE(Integer, ExpDec, 67, "0d385875_437_1", {3858754371})
TEST_CASE(Integer, ExpDec, 68, "0d390468_751_4", {3904687514})
TEST_CASE(Integer, ExpDec, 69, "0d396766_826_9", {3967668269})
TEST_CASE(Integer, ExpDec, 70, "0d8812_258_47", {881225847})
TEST_CASE(Integer, ExpDec, 71, "0d8099_875_20", {809987520})
TEST_CASE(Integer, ExpDec, 72, "0d102369_154_5", {1023691545})
TEST_CASE(Integer, ExpDec, 73, "0d9692_340_94", {969234094})
TEST_CASE(Integer, ExpDec, 74, "0d6628_328_11", {662832811})
TEST_CASE(Integer, ExpDec, 75, "0d5916_004_12", {591600412})
TEST_CASE(Integer, ExpDec, 76, "0d7717_677_49", {771767749})
TEST_CASE(Integer, ExpDec, 77, "0d7172_998_26", {717299826})
TEST_CASE(Integer, ExpDec, 78, "0d3113_363_99", {311336399})
TEST_CASE(Integer, ExpDec, 79, "0d3743_089_84", {374308984})
TEST_CASE(Integer, ExpDec, 80, "0d4538_139_21", {453813921})
TEST_CASE(Integer, ExpDec, 81, "0d5335_764_70", {533576470})
TEST_CASE(Integer, ExpDec, 82, "0d25_881_363", {25881363})
TEST_CASE(Integer, ExpDec, 83, "0d88_864_420", {88864420})
TEST_CASE(Integer, ExpDec, 84, "0d1347_953_89", {134795389})
TEST_CASE(Integer, ExpDec, 85, "0d2145_520_10", {214552010})
TEST_CASE(Integer, ExpDec, 86, "0d202320_563_9", {2023205639})
TEST_CASE(Integer, ExpDec, 87, "0d208605_764_8", {2086057648})
TEST_CASE(Integer, ExpDec, 88, "0d189723_863_3", {1897238633})
TEST_CASE(Integer, ExpDec, 89, "0d197686_422_2", {1976864222})
TEST_CASE(Integer, ExpDec, 90, "0d180485_269_9", {1804852699})
TEST_CASE(Integer, ExpDec, 91, "0d186769_418_8", {1867694188})
TEST_CASE(Integer, ExpDec, 92, "0d164534_034_1", {1645340341})
TEST_CASE(Integer, ExpDec, 93, "0d172497_177_8", {1724971778})
TEST_CASE(Integer, ExpDec, 94, "0d158749_663_9", {1587496639})
TEST_CASE(Integer, ExpDec, 95, "0d151613_312_8", {1516133128})
TEST_CASE(Integer, ExpDec, 96, "0d146155_054_5", {1461550545})
TEST_CASE(Integer, ExpDec, 97, "0d140695_152_6", {1406951526})
TEST_CASE(Integer, ExpDec, 98, "0d130201_609_9", {1302016099})
TEST_CASE(Integer, ExpDec, 99, "0d123064_674_0", {1230646740})
TEST_CASE(Integer, ExpDec, 100, "0d114249_191_7", {1142491917})

///=============================================================================
/// HEXADECIMAL LITERALS
TEST_CASE(Integer, Hex, 0, "0x1 0x2 0x3", {0x1, 0x2, 0x3})
TEST_CASE(Integer, Hex, 1, "0X111 0X222 0x333", {0x111, 0x222, 0x333})
TEST_CASE(Integer, Hex, 2, "0x111111 222222 0x333333", {0x111111, 222222, 0x333333})
TEST_CASE(Integer, Hex, 3, "0xffffffffffffffffffffffffffffffff",
          {Token(IntL, "340282366920938463463374607431768211455")})
TEST_CASE(Integer, Hex, 4, "0x100000000000000000000000000000000", {});
TEST_CASE(Integer, Hex, 5, "0x123i32", {0x123, Token(Name, "i32")})
TEST_CASE(Integer, Hex, 6, "0x0", {0x0})
TEST_CASE(Integer, Hex, 7, "0x4c11_d_b7", {0x4c11db7})
TEST_CASE(Integer, Hex, 8, "0x9823_b_6e", {0x9823b6e})
TEST_CASE(Integer, Hex, 9, "0xd432_6_d9", {0xd4326d9})
TEST_CASE(Integer, Hex, 10, "0x130_476d_c", {0x130476dc})
TEST_CASE(Integer, Hex, 11, "0x17c_56b6_b", {0x17c56b6b})
TEST_CASE(Integer, Hex, 12, "0x1a8_64db_2", {0x1a864db2})
TEST_CASE(Integer, Hex, 13, "0x1e4_7500_5", {0x1e475005})
TEST_CASE(Integer, Hex, 14, "0x260_8edb_8", {0x2608edb8})
TEST_CASE(Integer, Hex, 15, "0x22c_9f00_f", {0x22c9f00f})
TEST_CASE(Integer, Hex, 16, "0x2f8_ad6d_6", {0x2f8ad6d6})
TEST_CASE(Integer, Hex, 17, "0x2b4_bcb6_1", {0x2b4bcb61})
TEST_CASE(Integer, Hex, 18, "0x350_c9b6_4", {0x350c9b64})
TEST_CASE(Integer, Hex, 19, "0x31c_d86d_3", {0x31cd86d3})
TEST_CASE(Integer, Hex, 20, "0x3c8_ea00_a", {0x3c8ea00a})
TEST_CASE(Integer, Hex, 21, "0x384_fbdb_d", {0x384fbdbd})
TEST_CASE(Integer, Hex, 22, "0x4c1_1db7_0", {0x4c11db70})
TEST_CASE(Integer, Hex, 23, "0x48d_0c6c_7", {0x48d0c6c7})
TEST_CASE(Integer, Hex, 24, "0x459_3e01_e", {0x4593e01e})
TEST_CASE(Integer, Hex, 25, "0x415_2fda_9", {0x4152fda9})
TEST_CASE(Integer, Hex, 26, "0x5f1_5ada_c", {0x5f15adac})
TEST_CASE(Integer, Hex, 27, "0x5bd_4b01_b", {0x5bd4b01b})
TEST_CASE(Integer, Hex, 28, "0x569_796c_2", {0x569796c2})
TEST_CASE(Integer, Hex, 29, "0x525_68b7_5", {0x52568b75})
TEST_CASE(Integer, Hex, 30, "0x6a1_936c_8", {0x6a1936c8})
TEST_CASE(Integer, Hex, 31, "0x6ed_82b7_f", {0x6ed82b7f})
TEST_CASE(Integer, Hex, 32, "0x639_b0da_6", {0x639b0da6})
TEST_CASE(Integer, Hex, 33, "0x675_a101_1", {0x675a1011})
TEST_CASE(Integer, Hex, 34, "0x791_d401_4", {0x791d4014})
TEST_CASE(Integer, Hex, 35, "0x7dd_c5da_3", {0x7ddc5da3})
TEST_CASE(Integer, Hex, 36, "0x709_f7b7_a", {0x709f7b7a})
TEST_CASE(Integer, Hex, 37, "0x745_e66c_d", {0x745e66cd})
TEST_CASE(Integer, Hex, 38, "0x982_3b6e_0", {0x9823b6e0})
TEST_CASE(Integer, Hex, 39, "0x9ce_2ab5_7", {0x9ce2ab57})
TEST_CASE(Integer, Hex, 40, "0x91a_18d8_e", {0x91a18d8e})
TEST_CASE(Integer, Hex, 41, "0x956_0903_9", {0x95609039})
TEST_CASE(Integer, Hex, 42, "0x8b2_7c03_c", {0x8b27c03c})
TEST_CASE(Integer, Hex, 43, "0x8fe_6dd8_b", {0x8fe6dd8b})
TEST_CASE(Integer, Hex, 44, "0x82a_5fb5_2", {0x82a5fb52})
TEST_CASE(Integer, Hex, 45, "0x866_4e6e_5", {0x8664e6e5})
TEST_CASE(Integer, Hex, 46, "0xbe2_b5b5_8", {0xbe2b5b58})
TEST_CASE(Integer, Hex, 47, "0xbae_a46e_f", {0xbaea46ef})
TEST_CASE(Integer, Hex, 48, "0xb7a_9603_6", {0xb7a96036})
TEST_CASE(Integer, Hex, 49, "0xb36_87d8_1", {0xb3687d81})
TEST_CASE(Integer, Hex, 50, "0xad2_f2d8_4", {0xad2f2d84})
TEST_CASE(Integer, Hex, 51, "0xa9e_e303_3", {0xa9ee3033})
TEST_CASE(Integer, Hex, 52, "0xa4a_d16e_a", {0xa4ad16ea})
TEST_CASE(Integer, Hex, 53, "0xa06_c0b5_d", {0xa06c0b5d})
TEST_CASE(Integer, Hex, 54, "0xd43_26d9_0", {0xd4326d90})
TEST_CASE(Integer, Hex, 55, "0xd0f_3702_7", {0xd0f37027})
TEST_CASE(Integer, Hex, 56, "0xddb_056f_e", {0xddb056fe})
TEST_CASE(Integer, Hex, 57, "0xd97_14b4_9", {0xd9714b49})
TEST_CASE(Integer, Hex, 58, "0xc73_61b4_c", {0xc7361b4c})
TEST_CASE(Integer, Hex, 59, "0xc3f_706f_b", {0xc3f706fb})
TEST_CASE(Integer, Hex, 60, "0xceb_4202_2", {0xceb42022})
TEST_CASE(Integer, Hex, 61, "0xca7_53d9_5", {0xca753d95})
TEST_CASE(Integer, Hex, 62, "0xf23_a802_8", {0xf23a8028})
TEST_CASE(Integer, Hex, 63, "0xf6f_b9d9_f", {0xf6fb9d9f})
TEST_CASE(Integer, Hex, 64, "0xfbb_8bb4_6", {0xfbb8bb46})
TEST_CASE(Integer, Hex, 65, "0xff7_9a6f_1", {0xff79a6f1})
TEST_CASE(Integer, Hex, 66, "0xe13_ef6f_4", {0xe13ef6f4})
TEST_CASE(Integer, Hex, 67, "0xe5f_feb4_3", {0xe5ffeb43})
TEST_CASE(Integer, Hex, 68, "0xe8b_ccd9_a", {0xe8bccd9a})
TEST_CASE(Integer, Hex, 69, "0xec7_dd02_d", {0xec7dd02d})
TEST_CASE(Integer, Hex, 70, "0x348_6707_7", {0x34867077})
TEST_CASE(Integer, Hex, 71, "0x304_76dc_0", {0x30476dc0})
TEST_CASE(Integer, Hex, 72, "0x3d0_44b1_9", {0x3d044b19})
TEST_CASE(Integer, Hex, 73, "0x39c_556a_e", {0x39c556ae})
TEST_CASE(Integer, Hex, 74, "0x278_206a_b", {0x278206ab})
TEST_CASE(Integer, Hex, 75, "0x234_31b1_c", {0x23431b1c})
TEST_CASE(Integer, Hex, 76, "0x2e0_03dc_5", {0x2e003dc5})
TEST_CASE(Integer, Hex, 77, "0x2ac_1207_2", {0x2ac12072})
TEST_CASE(Integer, Hex, 78, "0x128_e9dc_f", {0x128e9dcf})
TEST_CASE(Integer, Hex, 79, "0x164_f807_8", {0x164f8078})
TEST_CASE(Integer, Hex, 80, "0x1b0_ca6a_1", {0x1b0ca6a1})
TEST_CASE(Integer, Hex, 81, "0x1fc_dbb1_6", {0x1fcdbb16})
TEST_CASE(Integer, Hex, 82, "0x18a_eb_13", {0x18aeb13})
TEST_CASE(Integer, Hex, 83, "0x54b_f6_a4", {0x54bf6a4})
TEST_CASE(Integer, Hex, 84, "0x808_d0_7d", {0x808d07d})
TEST_CASE(Integer, Hex, 85, "0xcc9_cd_ca", {0xcc9cdca})
TEST_CASE(Integer, Hex, 86, "0x789_7ab0_7", {0x7897ab07})
TEST_CASE(Integer, Hex, 87, "0x7c5_6b6b_0", {0x7c56b6b0})
TEST_CASE(Integer, Hex, 88, "0x711_5906_9", {0x71159069})
TEST_CASE(Integer, Hex, 89, "0x75d_48dd_e", {0x75d48dde})
TEST_CASE(Integer, Hex, 90, "0x6b9_3ddd_b", {0x6b93dddb})
TEST_CASE(Integer, Hex, 91, "0x6f5_2c06_c", {0x6f52c06c})
TEST_CASE(Integer, Hex, 92, "0x621_1e6b_5", {0x6211e6b5})
TEST_CASE(Integer, Hex, 93, "0x66d_0fb0_2", {0x66d0fb02})
TEST_CASE(Integer, Hex, 94, "0x5e9_f46b_f", {0x5e9f46bf})
TEST_CASE(Integer, Hex, 95, "0x5a5_e5b0_8", {0x5a5e5b08})
TEST_CASE(Integer, Hex, 96, "0x571_d7dd_1", {0x571d7dd1})
TEST_CASE(Integer, Hex, 97, "0x53d_c606_6", {0x53dc6066})
TEST_CASE(Integer, Hex, 98, "0x4d9_b306_3", {0x4d9b3063})
TEST_CASE(Integer, Hex, 99, "0x495_a2dd_4", {0x495a2dd4})
TEST_CASE(Integer, Hex, 100, "0x4419_0b0_d", {0x44190b0d})

///=============================================================================
/// OCTAL LITERALS
TEST_CASE(Integer, Oct, 0, "0o1 0o2 0o3", {1, 2, 3})
TEST_CASE(Integer, Oct, 1, "0O111 0O222 0o333", {73, 146, 219})
TEST_CASE(Integer, Oct, 2, "0o111111 222222 0o333333", {37449, 222222, 112347})
TEST_CASE(Integer, Oct, 3, "0o3777777777777777777777777777777777777777777",
          {Token(IntL, "340282366920938463463374607431768211455")})
TEST_CASE(Integer, Oct, 4, "0o4000000000000000000000000000000000000000000", {});
TEST_CASE(Integer, Oct, 5, "0o123i32", {83, Token(Name, "i32")})
TEST_CASE(Integer, Oct, 6, "0o0", {0x0})
TEST_CASE(Integer, Oct, 7, "0o460_216_696", {})
TEST_CASE(Integer, Oct, 8, "0o114_0435_556", {0x9823b6e})
TEST_CASE(Integer, Oct, 9, "0o152_0623_331", {0xd4326d9})
TEST_CASE(Integer, Oct, 10, "0o23_010733_34", {0x130476dc})
TEST_CASE(Integer, Oct, 11, "0o27_612655_53", {0x17c56b6b})
TEST_CASE(Integer, Oct, 12, "0o32_414466_62", {0x1a864db2})
TEST_CASE(Integer, Oct, 13, "0o36_216500_05", {0x1e475005})
TEST_CASE(Integer, Oct, 14, "0o46_021666_70", {0x2608edb8})
TEST_CASE(Integer, Oct, 15, "0o42_623700_17", {0x22c9f00f})
TEST_CASE(Integer, Oct, 16, "0o57_425533_26", {0x2f8ad6d6})
TEST_CASE(Integer, Oct, 17, "0o53_227455_41", {0x2b4bcb61})
TEST_CASE(Integer, Oct, 18, "0o65_031155_44", {0x350c9b64})
TEST_CASE(Integer, Oct, 19, "0o61_633033_23", {0x31cd86d3})
TEST_CASE(Integer, Oct, 20, "0o74_435200_12", {0x3c8ea00a})
TEST_CASE(Integer, Oct, 21, "0o70_237366_75", {0x384fbdbd})
TEST_CASE(Integer, Oct, 22, "0o11_4043555_60", {0x4c11db70})
TEST_CASE(Integer, Oct, 23, "0o11_0641433_07", {0x48d0c6c7})
TEST_CASE(Integer, Oct, 24, "0o10_5447600_36", {0x4593e01e})
TEST_CASE(Integer, Oct, 25, "0o10_1245766_51", {0x4152fda9})
TEST_CASE(Integer, Oct, 26, "0o13_7053266_54", {0x5f15adac})
TEST_CASE(Integer, Oct, 27, "0o13_3651300_33", {0x5bd4b01b})
TEST_CASE(Integer, Oct, 28, "0o12_6457133_02", {0x569796c2})
TEST_CASE(Integer, Oct, 29, "0o12_2255055_65", {0x52568b75})
TEST_CASE(Integer, Oct, 30, "0o15_2062333_10", {0x6a1936c8})
TEST_CASE(Integer, Oct, 31, "0o15_6660255_77", {0x6ed82b7f})
TEST_CASE(Integer, Oct, 32, "0o14_3466066_46", {0x639b0da6})
TEST_CASE(Integer, Oct, 33, "0o14_7264100_21", {0x675a1011})
TEST_CASE(Integer, Oct, 34, "0o17_1072400_24", {0x791d4014})
TEST_CASE(Integer, Oct, 35, "0o17_5670566_43", {0x7ddc5da3})
TEST_CASE(Integer, Oct, 36, "0o16_0476755_72", {0x709f7b7a})
TEST_CASE(Integer, Oct, 37, "0o16_4274633_15", {0x745e66cd})
TEST_CASE(Integer, Oct, 38, "0o23_0107333_40", {0x9823b6e0})
TEST_CASE(Integer, Oct, 39, "0o23_4705255_27", {0x9ce2ab57})
TEST_CASE(Integer, Oct, 40, "0o22_1503066_16", {0x91a18d8e})
TEST_CASE(Integer, Oct, 41, "0o22_5301100_71", {0x95609039})
TEST_CASE(Integer, Oct, 42, "0o21_3117400_74", {0x8b27c03c})
TEST_CASE(Integer, Oct, 43, "0o21_7715566_13", {0x8fe6dd8b})
TEST_CASE(Integer, Oct, 44, "0o20_2513755_22", {0x82a5fb52})
TEST_CASE(Integer, Oct, 45, "0o20_6311633_45", {0x8664e6e5})
TEST_CASE(Integer, Oct, 46, "0o27_6126555_30", {0xbe2b5b58})
TEST_CASE(Integer, Oct, 47, "0o27_2724433_57", {0xbaea46ef})
TEST_CASE(Integer, Oct, 48, "0o26_7522600_66", {0xb7a96036})
TEST_CASE(Integer, Oct, 49, "0o26_3320766_01", {0xb3687d81})
TEST_CASE(Integer, Oct, 50, "0o25_5136266_04", {0xad2f2d84})
TEST_CASE(Integer, Oct, 51, "0o25_1734300_63", {0xa9ee3033})
TEST_CASE(Integer, Oct, 52, "0o24_4532133_52", {0xa4ad16ea})
TEST_CASE(Integer, Oct, 53, "0o24_0330055_35", {0xa06c0b5d})
TEST_CASE(Integer, Oct, 54, "0o32_4144666_20", {0xd4326d90})
TEST_CASE(Integer, Oct, 55, "0o32_0746700_47", {0xd0f37027})
TEST_CASE(Integer, Oct, 56, "0o33_5540533_76", {0xddb056fe})
TEST_CASE(Integer, Oct, 57, "0o33_1342455_11", {0xd9714b49})
TEST_CASE(Integer, Oct, 58, "0o30_7154155_14", {0xc7361b4c})
TEST_CASE(Integer, Oct, 59, "0o30_3756033_73", {0xc3f706fb})
TEST_CASE(Integer, Oct, 60, "0o31_6550200_42", {0xceb42022})
TEST_CASE(Integer, Oct, 61, "0o31_2352366_25", {0xca753d95})
TEST_CASE(Integer, Oct, 62, "0o36_2165000_50", {0xf23a8028})
TEST_CASE(Integer, Oct, 63, "0o36_6767166_37", {0xf6fb9d9f})
TEST_CASE(Integer, Oct, 64, "0o37_3561355_06", {0xfbb8bb46})
TEST_CASE(Integer, Oct, 65, "0o37_7363233_61", {0xff79a6f1})
TEST_CASE(Integer, Oct, 66, "0o34_1175733_64", {0xe13ef6f4})
TEST_CASE(Integer, Oct, 67, "0o34_5777655_03", {0xe5ffeb43})
TEST_CASE(Integer, Oct, 68, "0o35_0571466_32", {0xe8bccd9a})
TEST_CASE(Integer, Oct, 69, "0o35_4373500_55", {0xec7dd02d})
TEST_CASE(Integer, Oct, 70, "0o64_414701_67", {0x34867077})
TEST_CASE(Integer, Oct, 71, "0o60_216667_00", {0x30476dc0})
TEST_CASE(Integer, Oct, 72, "0o75_010454_31", {0x3d044b19})
TEST_CASE(Integer, Oct, 73, "0o71_612532_56", {0x39c556ae})
TEST_CASE(Integer, Oct, 74, "0o47_404032_53", {0x278206ab})
TEST_CASE(Integer, Oct, 75, "0o43_206154_34", {0x23431b1c})
TEST_CASE(Integer, Oct, 76, "0o56_000367_05", {0x2e003dc5})
TEST_CASE(Integer, Oct, 77, "0o52_602201_62", {0x2ac12072})
TEST_CASE(Integer, Oct, 78, "0o22_435167_17", {0x128e9dcf})
TEST_CASE(Integer, Oct, 79, "0o26_237001_70", {0x164f8078})
TEST_CASE(Integer, Oct, 80, "0o33_031232_41", {0x1b0ca6a1})
TEST_CASE(Integer, Oct, 81, "0o37_633354_26", {0x1fcdbb16})
TEST_CASE(Integer, Oct, 82, "0o14_2565_423", {0x18aeb13})
TEST_CASE(Integer, Oct, 83, "0o52_2773_244", {0x54bf6a4})
TEST_CASE(Integer, Oct, 84, "0o10_02150_175", {0x808d07d})
TEST_CASE(Integer, Oct, 85, "0o14_62346_712", {0xcc9cdca})
TEST_CASE(Integer, Oct, 86, "0o17_0457254_07", {0x7897ab07})
TEST_CASE(Integer, Oct, 87, "0o17_4255332_60", {0x7c56b6b0})
TEST_CASE(Integer, Oct, 88, "0o16_1053101_51", {0x71159069})
TEST_CASE(Integer, Oct, 89, "0o16_5651067_36", {0x75d48dde})
TEST_CASE(Integer, Oct, 90, "0o15_3447567_33", {0x6b93dddb})
TEST_CASE(Integer, Oct, 91, "0o15_7245401_54", {0x6f52c06c})
TEST_CASE(Integer, Oct, 92, "0o14_2043632_65", {0x6211e6b5})
TEST_CASE(Integer, Oct, 93, "0o14_6641754_02", {0x66d0fb02})
TEST_CASE(Integer, Oct, 94, "0o13_6476432_77", {0x5e9f46bf})
TEST_CASE(Integer, Oct, 95, "0o13_2274554_10", {0x5a5e5b08})
TEST_CASE(Integer, Oct, 96, "0o12_7072767_21", {0x571d7dd1})
TEST_CASE(Integer, Oct, 97, "0o12_3670601_46", {0x53dc6066})
TEST_CASE(Integer, Oct, 98, "0o11_5466301_43", {0x4d9b3063})
TEST_CASE(Integer, Oct, 99, "0o11_1264267_24", {0x495a2dd4})
TEST_CASE(Integer, Oct, 100, "0o1_04062054_15", {0x44190b0d})

///=============================================================================
/// BINARY LITERALS
TEST_CASE(Integer, Bin, 0, "0b10 0b01 0b101", {2, 1, 5})
TEST_CASE(Integer, Bin, 2, "0b111111 222222 0B1011101", {0b111111, 222222, 0B1011101})
TEST_CASE(Integer, Bin, 3,
          "0b1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
          "1111111111111111111111",
          {Token(IntL, "340282366920938463463374607431768211455")})
TEST_CASE(Integer, Bin, 4,
          "0b1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000",
          {});
TEST_CASE(Integer, Bin, 5, "0b10110i32", {22, Token(Name, "i32")})
TEST_CASE(Integer, Bin, 6, "0b0", {0b0})
TEST_CASE(Integer, Bin, 7, "0b100_11_000001000c1_10110_110111", {})
TEST_CASE(Integer, Bin, 8, "0b_1001_1000001000111_01101_101110", {0b1001100000100011101101101110})
TEST_CASE(Integer, Bin, 9, "0b_1101_0100001100100_11011_011001", {0b1101010000110010011011011001})
TEST_CASE(Integer, Bin, 10, "0b1_0011_0000010001110_11011_011100", {0b10011000001000111011011011100})
TEST_CASE(Integer, Bin, 11, "0b1_0111_1100010101101_01101_101011", {0b10111110001010110101101101011})
TEST_CASE(Integer, Bin, 12, "0b1_1010_1000011001001_10110_110010", {0b11010100001100100110110110010})
TEST_CASE(Integer, Bin, 13, "0b1_1110_0100011101010_00000_000101", {0b11110010001110101000000000101})
TEST_CASE(Integer, Bin, 14, "0b10_0110_0000100011101_10110_111000", {0b100110000010001110110110111000})
TEST_CASE(Integer, Bin, 15, "0b10_0010_1100100111110_00000_001111", {0b100010110010011111000000001111})
TEST_CASE(Integer, Bin, 16, "0b10_1111_1000101011010_11011_010110", {0b101111100010101101011011010110})
TEST_CASE(Integer, Bin, 17, "0b10_1011_0100101111001_01101_100001", {0b101011010010111100101101100001})
TEST_CASE(Integer, Bin, 18, "0b11_0101_0000110010011_01101_100100", {0b110101000011001001101101100100})
TEST_CASE(Integer, Bin, 19, "0b11_0001_1100110110000_11011_010011", {0b110001110011011000011011010011})
TEST_CASE(Integer, Bin, 20, "0b11_1100_1000111010100_00000_001010", {0b111100100011101010000000001010})
TEST_CASE(Integer, Bin, 21, "0b11_1000_0100111110111_10110_111101", {0b111000010011111011110110111101})
TEST_CASE(Integer, Bin, 22, "0b100_1100_0001000111011_01101_110000", {0b1001100000100011101101101110000})
TEST_CASE(Integer, Bin, 23, "0b100_1000_1101000011000_11011_000111", {0b1001000110100001100011011000111})
TEST_CASE(Integer, Bin, 24, "0b100_0101_1001001111100_00000_011110", {0b1000101100100111110000000011110})
TEST_CASE(Integer, Bin, 25, "0b100_0001_0101001011111_10110_101001", {0b1000001010100101111110110101001})
TEST_CASE(Integer, Bin, 26, "0b101_1111_0001010110101_10110_101100", {0b1011111000101011010110110101100})
TEST_CASE(Integer, Bin, 27, "0b101_1011_1101010010110_00000_011011", {0b1011011110101001011000000011011})
TEST_CASE(Integer, Bin, 28, "0b101_0110_1001011110010_11011_000010", {0b1010110100101111001011011000010})
TEST_CASE(Integer, Bin, 29, "0b101_0010_0101011010001_01101_110101", {0b1010010010101101000101101110101})
TEST_CASE(Integer, Bin, 30, "0b110_1010_0001100100110_11011_001000", {0b1101010000110010011011011001000})
TEST_CASE(Integer, Bin, 31, "0b110_1110_1101100000101_01101_111111", {0b1101110110110000010101101111111})
TEST_CASE(Integer, Bin, 32, "0b110_0011_1001101100001_10110_100110", {0b1100011100110110000110110100110})
TEST_CASE(Integer, Bin, 33, "0b110_0111_0101101000010_00000_010001", {0b1100111010110100001000000010001})
TEST_CASE(Integer, Bin, 34, "0b111_1001_0001110101000_00000_010100", {0b1111001000111010100000000010100})
TEST_CASE(Integer, Bin, 35, "0b111_1101_1101110001011_10110_100011", {0b1111101110111000101110110100011})
TEST_CASE(Integer, Bin, 36, "0b111_0000_1001111101111_01101_111010", {0b1110000100111110111101101111010})
TEST_CASE(Integer, Bin, 37, "0b111_0100_0101111001100_11011_001101", {0b1110100010111100110011011001101})
TEST_CASE(Integer, Bin, 38, "0b1001_1000_0010001110110_11011_100000", {0b10011000001000111011011011100000})
TEST_CASE(Integer, Bin, 39, "0b1001_1100_1110001010101_01101_010111", {0b10011100111000101010101101010111})
TEST_CASE(Integer, Bin, 40, "0b1001_0001_1010000110001_10110_001110", {0b10010001101000011000110110001110})
TEST_CASE(Integer, Bin, 41, "0b1001_0101_0110000010010_00000_111001", {0b10010101011000001001000000111001})
TEST_CASE(Integer, Bin, 42, "0b1000_1011_0010011111000_00000_111100", {0b10001011001001111100000000111100})
TEST_CASE(Integer, Bin, 43, "0b1000_1111_1110011011011_10110_001011", {0b10001111111001101101110110001011})
TEST_CASE(Integer, Bin, 44, "0b1000_0010_1010010111111_01101_010010", {0b10000010101001011111101101010010})
TEST_CASE(Integer, Bin, 45, "0b1000_0110_0110010011100_11011_100101", {0b10000110011001001110011011100101})
TEST_CASE(Integer, Bin, 46, "0b1011_1110_0010101101011_01101_011000", {0b10111110001010110101101101011000})
TEST_CASE(Integer, Bin, 47, "0b1011_1010_1110101001000_11011_101111", {0b10111010111010100100011011101111})
TEST_CASE(Integer, Bin, 48, "0b1011_0111_1010100101100_00000_110110", {0b10110111101010010110000000110110})
TEST_CASE(Integer, Bin, 49, "0b1011_0011_0110100001111_10110_000001", {0b10110011011010000111110110000001})
TEST_CASE(Integer, Bin, 50, "0b1010_1101_0010111100101_10110_000100", {0b10101101001011110010110110000100})
TEST_CASE(Integer, Bin, 51, "0b1010_1001_1110111000110_00000_110011", {0b10101001111011100011000000110011})
TEST_CASE(Integer, Bin, 52, "0b1010_0100_1010110100010_11011_101010", {0b10100100101011010001011011101010})
TEST_CASE(Integer, Bin, 53, "0b1010_0000_0110110000001_01101_011101", {0b10100000011011000000101101011101})
TEST_CASE(Integer, Bin, 54, "0b1101_0100_0011001001101_10110_010000", {0b11010100001100100110110110010000})
TEST_CASE(Integer, Bin, 55, "0b1101_0000_1111001101110_00000_100111", {0b11010000111100110111000000100111})
TEST_CASE(Integer, Bin, 56, "0b1101_1101_1011000001010_11011_111110", {0b11011101101100000101011011111110})
TEST_CASE(Integer, Bin, 57, "0b1101_1001_0111000101001_01101_001001", {0b11011001011100010100101101001001})
TEST_CASE(Integer, Bin, 58, "0b1100_0111_0011011000011_01101_001100", {0b11000111001101100001101101001100})
TEST_CASE(Integer, Bin, 59, "0b1100_0011_1111011100000_11011_111011", {0b11000011111101110000011011111011})
TEST_CASE(Integer, Bin, 60, "0b1100_1110_1011010000100_00000_100010", {0b11001110101101000010000000100010})
TEST_CASE(Integer, Bin, 61, "0b1100_1010_0111010100111_10110_010101", {0b11001010011101010011110110010101})
TEST_CASE(Integer, Bin, 62, "0b1111_0010_0011101010000_00000_101000", {0b11110010001110101000000000101000})
TEST_CASE(Integer, Bin, 63, "0b1111_0110_1111101110011_10110_011111", {0b11110110111110111001110110011111})
TEST_CASE(Integer, Bin, 64, "0b1111_1011_1011100010111_01101_000110", {0b11111011101110001011101101000110})
TEST_CASE(Integer, Bin, 65, "0b1111_1111_0111100110100_11011_110001", {0b11111111011110011010011011110001})
TEST_CASE(Integer, Bin, 66, "0b1110_0001_0011111011110_11011_110100", {0b11100001001111101111011011110100})
TEST_CASE(Integer, Bin, 67, "0b1110_0101_1111111111101_01101_000011", {0b11100101111111111110101101000011})
TEST_CASE(Integer, Bin, 68, "0b1110_1000_1011110011001_10110_011010", {0b11101000101111001100110110011010})
TEST_CASE(Integer, Bin, 69, "0b1110_1100_0111110111010_00000_101101", {0b11101100011111011101000000101101})
TEST_CASE(Integer, Bin, 70, "0b11_0100_1000011001110_00001_110111", {0b110100100001100111000001110111})
TEST_CASE(Integer, Bin, 71, "0b11_0000_0100011101101_10111_000000", {0b110000010001110110110111000000})
TEST_CASE(Integer, Bin, 72, "0b11_1101_0000010001001_01100_011001", {0b111101000001000100101100011001})
TEST_CASE(Integer, Bin, 73, "0b11_1001_1100010101010_11010_101110", {0b111001110001010101011010101110})
TEST_CASE(Integer, Bin, 74, "0b10_0111_1000001000000_11010_101011", {0b100111100000100000011010101011})
TEST_CASE(Integer, Bin, 75, "0b10_0011_0100001100011_01100_011100", {0b100011010000110001101100011100})
TEST_CASE(Integer, Bin, 76, "0b10_1110_0000000000111_10111_000101", {0b101110000000000011110111000101})
TEST_CASE(Integer, Bin, 77, "0b10_1010_1100000100100_00001_110010", {0b101010110000010010000001110010})
TEST_CASE(Integer, Bin, 78, "0b1_0010_1000111010011_10111_001111", {0b10010100011101001110111001111})
TEST_CASE(Integer, Bin, 79, "0b1_0110_0100111110000_00001_111000", {0b10110010011111000000001111000})
TEST_CASE(Integer, Bin, 80, "0b1_1011_0000110010100_11010_100001", {0b11011000011001010011010100001})
TEST_CASE(Integer, Bin, 81, "0b1_1111_1100110110111_01100_010110", {0b11111110011011011101100010110})
TEST_CASE(Integer, Bin, 82, "0b1_1000101011101_01100_010011", {0b1100010101110101100010011})
TEST_CASE(Integer, Bin, 83, "0b_101_0100101111110_11010_100100", {0b101010010111111011010100100})
TEST_CASE(Integer, Bin, 84, "0b_1000_0000100011010_00001_111101", {0b1000000010001101000001111101})
TEST_CASE(Integer, Bin, 85, "0b_1100_1100100111001_10111_001010", {0b1100110010011100110111001010})
TEST_CASE(Integer, Bin, 86, "0b111_1000_1001011110101_01100_000111", {0b1111000100101111010101100000111})
TEST_CASE(Integer, Bin, 87, "0b111_1100_0101011010110_11010_110000", {0b1111100010101101011011010110000})
TEST_CASE(Integer, Bin, 88, "0b111_0001_0001010110010_00001_101001", {0b1110001000101011001000001101001})
TEST_CASE(Integer, Bin, 89, "0b111_0101_1101010010001_10111_011110", {0b1110101110101001000110111011110})
TEST_CASE(Integer, Bin, 90, "0b110_1011_1001001111011_10111_011011", {0b1101011100100111101110111011011})
TEST_CASE(Integer, Bin, 91, "0b110_1111_0101001011000_00001_101100", {0b1101111010100101100000001101100})
TEST_CASE(Integer, Bin, 92, "0b110_0010_0001000111100_11010_110101", {0b1100010000100011110011010110101})
TEST_CASE(Integer, Bin, 93, "0b110_0110_1101000011111_01100_000010", {0b1100110110100001111101100000010})
TEST_CASE(Integer, Bin, 94, "0b101_1110_1001111101000_11010_111111", {0b1011110100111110100011010111111})
TEST_CASE(Integer, Bin, 95, "0b101_1010_0101111001011_01100_001000", {0b1011010010111100101101100001000})
TEST_CASE(Integer, Bin, 96, "0b101_0111_0001110101111_10111_010001", {0b1010111000111010111110111010001})
TEST_CASE(Integer, Bin, 97, "0b101_0011_1101110001100_00001_100110", {0b1010011110111000110000001100110})
TEST_CASE(Integer, Bin, 98, "0b100_1101_1001101100110_00001_100011", {0b1001101100110110011000001100011})
TEST_CASE(Integer, Bin, 99, "0b100_1001_0101101000101_10111_010100", {0b1001001010110100010110111010100})
TEST_CASE(Integer, Bin, 100, "0b100_0100_0001100100001_01100_001101", {0b1000100000110010000101100001101})
