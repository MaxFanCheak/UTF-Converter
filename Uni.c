#include <stdio.h>

void printRes(int iMax, char res[4], FILE* out);

int encoding(int mode, int encoding, char* symbols, char* result, int size);

int utf8ToUnicode(char* storage, int length);

int unicodeToUtf8(char* storage, int unicode);

int utf16leToUnicode(char* storage, int length);

int unicodeToUtf16le(char* storage, int unicode);

int utf16beToUnicode(char* storage, int length);

int unicodeToUtf16Be(char* storage, int unicode);

int utf32LeToUnicode(char* storage);

void unicodeToUtf32Le(char* storage, int unicode);

int utf32BeToUnicode(char* storage);

void unicodeToUtf32Be(char* storage, int unicode);

void utf8(int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char* coding);

void utf16(int mode, int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char BOM[4], char* coding);

void utf32(int mode, int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char* coding);

int checkBOM(char bom[4]);

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Wrong number of arguments");
		return 2;
	}
	char BOM[4];
	FILE* fin;
	if ((fin = fopen(argv[1], "r")) == 0)
	{
		printf("Invalid input file\n");
		return 2;
	}
	char* coding = argv[3];
	fscanf(fin, "%s", &BOM);
	FILE* fout;
	if ((fout = fopen(argv[2], "w")) == 0)
	{
		printf("Invalid output file\n");
		if (fclose(fin) == 0)
		{
			printf("Invalid input file\n");
		}
		return 2;
	}
	switch (*coding - '0')
	{
	case 0:
	case 1:
		fprintf(fout, "%c%c%c", 0xEF, 0xBB, 0xBF);
	case 2:
		fprintf(fout, "%c%c", 0xFF, 0xFE);
	case 3:
		fprintf(fout, "%c%c", 0xFE, 0xFF);
	case 4:
		fprintf(fout, "%c%c%c%c", 0xFF, 0xFE, 0x00, 0x00);
	case 5:
		fprintf(fout, "%c%c%c%c", 0x00, 0x00, 0xFE, 0xFF);
	}
	char sym[4];
	char storage[4];
	int countOfBytes = 0;
	int mode = checkBOM(BOM);
	switch (mode)
	{
	case 1:
	{
		utf8(countOfBytes, storage, fin, fout, sym, coding);
	}
	case 2:
	{
		utf16(mode, countOfBytes, storage, fin, fout, sym, BOM, coding);
	}
	case 3:
	{
		utf16(mode, countOfBytes, storage, fin, fout, sym, BOM, coding);
	}
	case 4:
	{
		utf32(mode, countOfBytes, storage, fin, fout, sym, coding);
	}
	case 5:
	{
		utf32(mode, countOfBytes, storage, fin, fout, sym, coding);
	}
	}
	if (fclose(fin) == EOF)
	{
		printf("Error of closing input file");
		if (fclose(fout) == EOF)
		{
			printf("Error of closing output file");
		}
		return 2;
	}
	if (fclose(fout) == EOF)
	{
		printf("Error of closing output file");
		return 2;
	}
	return 0;
}

int checkBOM(char bom[4])
{
	if (bom[0] == '\xFF' && bom[1] == '\xFE')
	{
		if (bom[3] == '\x00')
		{
			return 4;
		}
		else
		{
			return 2;
		}
	}
	else if (bom == "\x00\x00\xFE\xFF")
	{
		return 5;
	}
	else if (bom[0] == '\xFE' && bom[1] == '\xFF')
	{
		return 3;
	}
	else
	{
		return 1;
	}
}

int nothing(char* in, char* out, int size)
{
	out = in;
	return size;
}

void printRes(int iMax, char res[4], FILE* out)
{
	for (int i = 0; i < iMax; i++)
	{
		fprintf(out, "%c", res[i]);
	}
}

void utf8(int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char* coding)
{
	fseek(fin, 0L, SEEK_SET);
	fscanf(fin, "%c", &sym[0]);
	while (1)
	{
		if (sym[0] <= 0x01111111)
		{
			index = encoding(*coding - '0', 1, sym, storage, 1);
		}
		else if (sym[0] <= 0x11011111)
		{
			fscanf(fin, "%c", &sym[1]);
			sym[0] &= 0x00111111;
			sym[1] &= 0x01111111;
			index = encoding(*coding - '0', 1, sym, storage, 2);
		}
		else if (sym[0] <= 0x11101111)
		{
			fscanf(fin, "%c%c", &sym[1], &sym[2]);
			sym[0] &= 0x00011111;
			sym[1] &= 0x01111111;
			sym[2] &= 0x01111111;
			index = encoding(*coding - '0', 1, sym, storage, 3);
		}
		else if (sym[0] <= 0x11110111)
		{
			fscanf(fin, "%c%c%c", &sym[1], &sym[2], &sym[3]);
			sym[0] &= 0x00001111;
			sym[1] &= 0x01111111;
			sym[2] &= 0x01111111;
			sym[3] &= 0x01111111;
			index = encoding(*coding - '0', 1, sym, storage, 3);
		}
		printRes(index, storage, fout);
		fscanf(fin, "%c", &sym[0]);
		if (feof(fin))
		{
			break;
		}
		sym[1] = (sym[2] = (sym[3] = '\0'));
	}
}

void utf16(int mode, int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char BOM[4], char* coding)
{
	sym[0] = BOM[2];
	sym[1] = BOM[3];
	while (1)
	{
		if (utf16beToUnicode(sym, 2) <= 0xD7FF)
		{
			index = encoding(*coding - '0', mode, sym, storage, 2);
		}
		else
		{
			fscanf(fin, "%c%c", &sym[2], &sym[3]);
			index = encoding(*coding - '0', mode, sym, storage, 4);
		}
		printRes(index, storage, fout);
		fscanf(fin, "%c%c", &sym[0], &sym[1]);
		if (feof(fin))
		{
			break;
		}
		sym[2] = (sym[3] = '\0');
	}
}

void utf32(int mode, int index, char storage[4], FILE* fin, FILE* fout, char sym[4], char* coding)
{
	while (1)
	{
		sym[0] = fgetc(fin);
		if (feof(fin))
		{
			break;
		}
		sym[1] = fgetc(fin);
		sym[2] = fgetc(fin);
		sym[3] = fgetc(fin);
		index = encoding(*coding - '0', mode, sym, storage, 4);
		printRes(index, storage, fout);
	}
}

int utf8ToUnicode(char* storage, int length)
{
	switch (length)
	{
	case 1:
		return storage[0];
	case 2:
		return (storage[1] - 0xC0 << 6) + (storage[0] - 0x80);
	case 3:
		return (storage[2] - 0xE0 << 12) + (storage[1] - 0x80 << 6) + (storage[0] - 0x80);
	case 4:
		return (storage[3] - 0xF0 << 18) + (storage[2] - 0x80 << 12) + (storage[1] - 0x80 << 6) +
			   (storage[0] - 0x80);
	}
}

int unicodeToUtf8(char* storage, int unicode)
{
	if (unicode <= 0x007F)
	{
		storage[0] = unicode;
		return 1;
	}
	if (unicode <= 0x7FF)
	{
		storage[0] = 0xC0 | unicode >> 6;
		storage[1] = 0x80 | unicode & 0x3F;
		return 2;
	}
	if (unicode <= 0xFFFF)
	{
		storage[0] = 0xE0 | unicode >> 12;
		storage[1] = 0x80 | (unicode >> 6 & 0x3F);
		storage[2] = 0x80 | unicode & 0x3F;
		return 3;
	}
	if (unicode <= 0x10FFFF)
	{
		storage[0] = 0xF0 | unicode >> 18;
		storage[1] = 0x80 | (unicode >> 12 & 0x3F);
		storage[2] = 0x80 | (unicode >> 6 & 0x3F);
		storage[3] = 0x80 | unicode & 0x3F;
		return 4;
	}
	return -1;
}

int utf16leToUnicode(char* storage, int length)
{
	switch (length)
	{
	case 2:
		return (storage[1] << 8) + storage[0];
	case 4:
		return ((((storage[1] << 8) + storage[0]) - 0xD800) * 0x0400) +
			   (((storage[3] << 8) + storage[2]) - 0xDC00) + 0x10000;
	}
}

int unicodeToUtf16le(char* storage, int unicode)
{
	if (unicode <= 0xFFFF)
	{
		storage[1] = unicode >> 8;
		storage[0] = unicode << 24 >> 24;
		return 2;
	}
	if (unicode <= 0x10FFFF)
	{
		storage[3] = ((unicode - 0x10000) >> 10) + 0xD800 >> 8;
		storage[2] = (((unicode - 0x10000 >> 10) + 0xD800) << 24) >> 24;
		storage[1] = ((unicode - 0x10000 << 22 >> 22) + 0xDC00) >> 8;
		storage[0] = (((unicode - 0x10000 << 22 >> 22) + 0xDC00) << 24) >> 24;
		return 4;
	}
	return -1;
}

int utf16beToUnicode(char* storage, int length)
{
	switch (length)
	{
	case 2:
		return (storage[0] << 8) + storage[1];
	case 4:
		return ((((storage[0] << 8) + storage[1]) - 0xD800) * 0x0400) +
			   (((storage[2] << 8) + storage[3]) - 0xDC00) + 0x10000;
	}
}

int unicodeToUtf16Be(char* storage, int unicode)
{
	if (unicode <= 0xFFFF)
	{
		storage[0] = unicode >> 8;
		storage[1] = unicode << 24 >> 24;
		return 2;
	}
	if (unicode <= 0x10FFFF)
	{
		storage[0] = (((unicode - 0x10000) >> 10) + 0xD800) >> 8;
		storage[1] = ((((unicode - 0x10000) >> 10) + 0xD800) << 24) >> 24;
		storage[2] = (((unicode - 0x10000) << 22 >> 22) + 0xDC00) >> 8;
		storage[3] = ((((unicode - 0x10000) << 22 >> 22) + 0xDC00) << 24) >> 24;
		return 4;
	}
	return -1;
}

int utf32LeToUnicode(char* storage)
{
	return (storage[3] << 24) + (storage[2] << 16) + (storage[1] << 8) + storage[0];
}

void unicodeToUtf32Le(char* storage, int unicode)
{
	storage[3] = unicode >> 24;
	storage[2] = unicode << 8 >> 24;
	storage[1] = unicode << 16 >> 24;
	storage[0] = unicode << 24 >> 24;
}

int utf32BeToUnicode(char* storage)
{
	return (storage[0] << 24) + (storage[1] << 16) + (storage[2] << 8) + storage[3];
}

void unicodeToUtf32Be(char* storage, int unicode)
{
	storage[0] = unicode >> 24;
	storage[1] = unicode << 8 >> 24;
	storage[2] = unicode << 16 >> 24;
	storage[3] = unicode << 24 >> 24;
}

int Utf32BeToUtf8(char* utf32Be, char* utf8)
{
	int res = unicodeToUtf8(utf8, utf32BeToUnicode(utf32Be));
	if (res != -1)
	{
		return res;
	}
	else
	{
		return -1;
	}
}

int Utf32BeToUtf16Be(char* utf32Be, char* utf16Be)
{
	int res = unicodeToUtf16Be(utf16Be, utf32BeToUnicode(utf32Be));
	if (res != -1)
	{
		return res;
	}
	else
	{
		return -1;
	}
}

int Utf32BeToUtf16Le(char* utf32Be, char* utf16Le)
{
	int res = unicodeToUtf16le(utf16Le, utf32BeToUnicode(utf32Be));
	if (res != -1)
	{
		return res;
	}
	else
	{
		return -1;
	}
}

int Utf32BeToUtf32Le(char* utf32Le, char* utf32Be)
{
	unicodeToUtf32Le(utf32Le, utf32BeToUnicode(utf32Be));
	return 4;
}

int Utf32LeToUtf32Be(char* utf32Le, char* utf32Be)
{
	unicodeToUtf32Be(utf32Be, utf32LeToUnicode(utf32Le));
	return 4;
}

int Utf32LeToUtf16Be(char* utf32Le, char* utf16Be)
{
	char storage[4];
	Utf32LeToUtf32Be(utf32Le, storage);
	Utf32BeToUtf16Be(storage, utf16Be);
}

int Utf32LeToUtf16Le(char* utf32Le, char* utf16Le)
{
	char storage[4];
	Utf32LeToUtf32Be(utf32Le, storage);
	Utf32BeToUtf16Le(storage, utf16Le);
}

int Utf32LeToUtf8(char* utf32Le, char* utf8)
{
	char storage[4];
	Utf32LeToUtf32Be(utf32Le, storage);
	Utf32BeToUtf8(storage, utf8);
}

int Utf16BeToUtf32Be(char* utf32Be, char* utf16Be, int size)
{
	unicodeToUtf32Be(utf32Be, utf16beToUnicode(utf16Be, size));
	return 4;
}

int Utf16BeToUtf32Le(char* utf32Le, char* utf16Be, int size)
{
	char storage[4];
	Utf16BeToUtf32Be(storage, utf16Be, size);
	Utf32BeToUtf32Le(utf32Le, storage);
	return 4;
}

int Utf16BeToUtf16Le(char* utf16Le, char* utf16Be, int size)
{
	char storage[4];
	Utf16BeToUtf32Be(storage, utf16Be, size);
	return Utf32BeToUtf16Le(storage, utf16Le);
}

int Utf16BeToUtf8(char* utf8, char* utf16Be, int size)
{
	char storage[4];
	Utf16BeToUtf32Be(storage, utf16Be, size);
	return Utf32BeToUtf8(storage, utf8);
}

int Utf16LeToUtf32Be(char* utf32Be, char* utf16Le, int size)
{
	unicodeToUtf32Be(utf32Be, utf16leToUnicode(utf16Le, size));
	return 4;
}

int Utf16LeToUtf32Le(char* utf32Le, char* utf16Le, int size)
{
	char storage[4];
	Utf16LeToUtf32Be(storage, utf16Le, size);
	Utf32BeToUtf32Le(utf32Le, storage);
	return 4;
}

int Utf16LeToUtf16Be(char* utf16Be, char* utf16Le, int size)
{
	char storage[4];
	Utf16LeToUtf32Be(storage, utf16Le, size);
	return Utf32BeToUtf16Be(storage, utf16Be);
}

int Utf16LeToUtf8(char* utf8, char* utf16Le, int size)
{
	char storage[4];
	Utf16LeToUtf32Be(storage, utf16Le, size);
	return Utf32BeToUtf8(storage, utf8);
}

int Utf8ToUtf32Be(char* utf32Be, char* utf8, int size)
{
	unicodeToUtf32Be(utf32Be, utf8ToUnicode(utf8, size));
	return 4;
}

int Utf8ToUtf32Le(char* utf32Le, char* utf8, int size)
{
	char storage[4];
	Utf8ToUtf32Be(storage, utf8, size);
	Utf32BeToUtf32Le(utf32Le, storage);
	return 4;
}

int Utf8ToUtf16Be(char* utf16Be, char* utf8, int size)
{
	char storage[4];
	Utf8ToUtf32Be(storage, utf8, size);
	return Utf32BeToUtf16Be(storage, utf16Be);
}

int Utf8ToUtf16Le(char* utf16Le, char* utf8, int size)
{
	char storage[4];
	Utf8ToUtf32Be(storage, utf8, size);
	return Utf32BeToUtf16Le(storage, utf16Le);
}

int encoding(int mode, int encoding, char* symbols, char* result, int size)
{
	switch (mode)
	{
	case 0 || 1:
		return (encoding == 0 || encoding == 1) ? nothing(symbols, result, size) :
			   (encoding == 2 ? Utf16LeToUtf8(result, symbols, size) :
				(encoding == 3 ? Utf16BeToUtf8(result, symbols, size) :
				 (encoding == 4 ? Utf32LeToUtf8(result, symbols) :
				  Utf32BeToUtf8(result, symbols))));
	case 2:
		return (encoding == 0 || encoding == 1) ? Utf8ToUtf16Le(symbols, result, size) :
			   (encoding == 2 ? nothing(result, symbols, size) :
				(encoding == 3 ? Utf16BeToUtf16Le(result, symbols, size) :
				 (encoding == 4 ? Utf32LeToUtf16Le(result, symbols) :
				  Utf32BeToUtf16Le(result, symbols))));
	case 3:
		return (encoding == 0 || encoding == 1) ? Utf8ToUtf16Be(symbols, result, size) :
			   (encoding == 2 ? Utf16LeToUtf16Be(result, symbols, size) :
				(encoding == 3 ? nothing(result, symbols, size) :
				 (encoding == 4 ? Utf32LeToUtf16Be(result, symbols) :
				  Utf32BeToUtf16Be(result, symbols))));
	case 4:
		return (encoding == 0 || encoding == 1) ? Utf8ToUtf32Le(symbols, result, size) :
			   (encoding == 2 ? Utf16LeToUtf32Le(result, symbols, size) :
				(encoding == 3 ? Utf16BeToUtf32Le(result, symbols, size) :
				 (encoding == 4 ? nothing(result, symbols, size) :
				  Utf32BeToUtf32Le(result, symbols))));
	case 5:
		return (encoding == 0 || encoding == 1) ? Utf8ToUtf32Be(symbols, result, size) :
			   (encoding == 2 ? Utf16LeToUtf32Be(result, symbols, size) :
				(encoding == 3 ? Utf16BeToUtf32Be(result, symbols, size) :
				 (encoding == 4 ? Utf32LeToUtf32Be(result, symbols) :
				  nothing(result, symbols, size))));
	}
}