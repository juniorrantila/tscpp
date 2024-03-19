#pragma once
#include "./Parse.h"

ErrorOr<StringBuffer> codegen(Source, ParseTree const&);
