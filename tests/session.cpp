#include "session.hpp"

using RecvData = ::CSession::SRecvData;

bool NTests::CSession::SRecvData::sanitize() {
  RecvData data;
  constexpr beforeSanitize = " a\n";
  constexpr afterSanitize = "a";

  data.data = beforeSanitize;
  return data == afterSanitize;
}

bool NTests::CSession::SRecvData::isEmpty() {
  RecvData data1, data2, data3, data4;
  constexpr emptyWithSpace = "    ";
  constexpr emptyWithNewLine = "\n";
  constexpr spaceWithNewLine = "   \n   ";
  constexpr empty = "";

  data1.data = emptyWithSpace;
  data2.data = emptyWithNewLine;
  data3.data = spaceWithNewLine;
  data4.data = empty;

  return data1.isEmpty() && data2.isEmpty() && data3.isEmpty() && data4.isEmpty() && data5.isEmpty();
}
