#include <gtest/gtest.h>
#include <nitrate/code.h>

#include <string_view>

static std::string_view source_code = R"(fn main(args: [str]): i32 {
  // Check if the user provided a file
  retif !args, print("Usage: test <file>\n"), 1;

  /**
    * Open the file and read its content
    * 
    * The file is closed automatically when the variable goes out of scope
    */
  let file = std::open(args[0], "r");
  let content = file.read_all();
  file.close();

  let x = 1e43 - 0b1001010010;
  print(x, content.as_bytes().transmute("enHEX").join(":").as_str());

  ret 0;
}
)";

static std::string_view expected_json =
    R"ESCAPE([[3,"fn",1,1,1,3],[6,"main",1,4,1,8],[5,"(",1,8,1,9],[6,"args",1,9,1,14],[5,":",1,14,1,14],[5,"[",1,15,1,16],[6,"str",1,16,1,19],[5,"]",1,19,1,20],[5,")",1,20,1,21],[5,":",1,21,1,22],[6,"i32",1,23,1,26],[5,"{",1,27,1,28],[13," Check if the user provided a file",2,3,2,39],[3,"retif",3,3,3,8],[4,"!",3,9,3,11],[6,"args",3,11,3,14],[5,",",3,14,3,15],[6,"print",3,16,3,21],[5,"(",3,21,3,22],[9,"Usage: test <file>\n",3,22,3,44],[5,")",3,44,3,45],[5,",",3,45,3,46],[7,"1",3,47,3,48],[5,";",3,48,3,49],[13,"*\n    * Open the file and read its content\n    * \n    * The file is closed automatically when the variable goes out of scope\n    ",5,3,9,6],[3,"let",10,3,10,6],[6,"file",10,7,10,11],[4,"=",10,12,10,14],[6,"std::open",10,14,10,23],[5,"(",10,23,10,24],[6,"args",10,24,10,28],[5,"[",10,28,10,29],[7,"0",10,29,10,30],[5,"]",10,30,10,31],[5,",",10,31,10,32],[9,"r",10,33,10,36],[5,")",10,36,10,37],[5,";",10,37,10,38],[3,"let",11,3,11,6],[6,"content",11,7,11,14],[4,"=",11,15,11,17],[6,"file",11,17,11,21],[4,".",11,21,11,23],[6,"read_all",11,23,11,30],[5,"(",11,30,11,31],[5,")",11,31,11,32],[5,";",11,32,11,33],[6,"file",12,3,12,7],[4,".",12,7,12,9],[6,"close",12,9,12,13],[5,"(",12,13,12,14],[5,")",12,14,12,15],[5,";",12,15,12,16],[3,"let",14,3,14,6],[6,"x",14,7,14,8],[4,"=",14,9,14,11],[8,"10000000000000000139372116959414099130712064.00000000000000000000",14,11,14,15],[4,"-",14,16,14,18],[7,"594",14,18,14,30],[5,";",14,30,14,31],[6,"print",15,3,15,8],[5,"(",15,8,15,9],[6,"x",15,9,15,10],[5,",",15,10,15,11],[6,"content",15,12,15,19],[4,".",15,19,15,21],[6,"as_bytes",15,21,15,28],[5,"(",15,28,15,29],[5,")",15,29,15,30],[4,".",15,30,15,32],[6,"transmute",15,32,15,40],[5,"(",15,40,15,41],[9,"enHEX",15,41,15,48],[5,")",15,48,15,49],[4,".",15,49,15,51],[6,"join",15,51,15,54],[5,"(",15,54,15,55],[9,":",15,55,15,58],[5,")",15,58,15,59],[4,".",15,59,15,61],[6,"as_str",15,61,15,66],[5,"(",15,66,15,67],[5,")",15,67,15,68],[5,")",15,68,15,69],[5,";",15,69,15,70],[3,"ret",17,3,17,6],[7,"0",17,7,17,8],[5,";",17,8,17,9],[5,"}",18,1,18,2],[1,"",0,0,0,0]])ESCAPE";

TEST(LexerRoute, flag_use_json) {
  auto source = NIT_MEMOPEN(source_code.data(), source_code.size());
  ASSERT_NE(source, nullptr);

  char* output_buf = nullptr;
  size_t output_size = 0;
  auto output = NIT_OPEM_STREAM(&output_buf, &output_size);
  ASSERT_NE(output, nullptr);

  const char* options[] = {
      "lex",        /* Lexer route */
      "-fuse-json", /* Output as JSON */
      NULL,         /* End of options */
  };

  EXPECT_TRUE(nit_cc(source, output, nullptr, 0, options));

  nit_fclose(source);
  nit_fclose(output);

  std::string_view output_code(output_buf, output_size);

  EXPECT_EQ(output_code, expected_json);

  free(output_buf);
}
