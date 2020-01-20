//
//  main.cpp
//  gen_simd_ amalgamated
//
//  Created by Attila Kocsis on 19/01/2020.
//  Copyright © 2020 Attila Kocsis. All rights reserved.
//

#include <bx/file.h>

#include <iostream>

char* load(bx::StringView _filename)
{
	bx::FileReader in;
	bx::Error err;
	in.open(bx::FilePath(_filename), &err);
	
	if (!err.isOk())
	{
		printf("cannot open input file: %.*s\n", _filename.getLength(), _filename.getPtr());
		exit(-1);
	}
	
	int64_t fileSize = bx::getSize(&in);
	char* buffer = new char[fileSize + 1];
	
	bx::read(&in, buffer, (int32_t)fileSize);
	buffer[fileSize] = 0;
	
	return buffer;
}

void process(bx::WriterI* _writer, bx::StringView _filename)
{
	printf("processing '%.*s'\n", _filename.getLength(), _filename.getPtr());

	char* buffer = load(_filename);
	
	int32_t len = bx::strLen(buffer);
	char* temp = new char[len];
	bx::StringView normalized = bx::normalizeEolLf(temp, len, buffer);
	
	for(bx::LineReader lr(normalized); !lr.isDone();)
	{
		bx::StringView line = lr.next();

		bool writeLine = true;
		
		bx::StringView trimLine = bx::strLTrimSpace(line);
		if ( trimLine.getLength() > 0 && trimLine.getPtr()[0] == '#')
		{
			bx::StringView token = bx::strLTrimSpace(bx::StringView(trimLine.getPtr()+1, trimLine.getTerm()));
			
			if ( 0 == bx::strCmp(token, "include", 7))
			{
				bx::StringView included = bx::strLTrimSpace(bx::StringView(token.getPtr()+7, token.getTerm()));
				
				included = bx::StringView(included.getPtr()+1, included.getTerm());
				
				bx::StringView term = bx::strFind(included, '"');
				
				if (!term.isEmpty())
				{
					included = bx::StringView(included.getPtr(), term.getPtr());
					printf("including '%.*s' in '%.*s' \n", included.getLength(), included.getPtr(), _filename.getLength(), _filename.getPtr());
					
					if (0 == bx::strCmp(included, "bx.h"))
					{
						if ( 0 != bx::strCmp(_filename, "macros.h"))
						{
							process(_writer, "platform.h");
							process(_writer, "macros.h");
						}
					}
					else
					{
						// add path
						bx::StringView path = bx::strRFind(_filename, '/');
						if (!path.isEmpty())
						{
							path = bx::StringView(_filename.getPtr(), path.getTerm());
						}
						
						std::string includedFullPath;
						bx::stringPrintf(includedFullPath, "%.*s%.*s", path.getLength(), path.getPtr(),
										 included.getLength(), included.getPtr());
						
						process(_writer, includedFullPath.c_str());
					}

					writeLine = false;
				}
			}
		}
		
		if ( writeLine )
		{
			bx::write(_writer, line);
			if (!(bx::strLen(line) == 1 && line.getPtr()[0] == 10))
			{
				bx::write(_writer, "\n");
			}
		}
	}
	
}

int main(int argc, const char * argv[]) {
	
	bx::FileWriter out;
	bx::Error err;
	out.open("bx_simd_t.h", false, &err);
	
	if (!err.isOk())
	{
		printf("cannot open output file\n");
		return -1;
	}

	process(&out, "simd_t.h");
	
	out.close();
	
	return 0;
}
