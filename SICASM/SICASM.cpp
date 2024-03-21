#include<fstream>
#include<cstring>
#include<sstream>
#include<iostream>
#include<vector>
#include<iomanip>
#include<list>
#include<stdlib.h>
#include<vector>

using namespace std;

fstream input, optab, outputL, outputR, outputS;
int locationCounter = 0,
startAddr = 0,
Expline = 0;

vector<string> buffer, bufferop;

class SymbolTable
{
public:
	string symbol;
	int address;
	list <int> preRef;

	SymbolTable(string theSymbol, int theAddress)
	{
		this->symbol = theSymbol;
		this->address = theAddress;
	}

	SymbolTable(string theSymbol, list<int> thePreRef)
	{
		this->symbol = theSymbol;
		this->address = -1; // 還未定義的標籤，給予-1的地址作為標記
		this->preRef = thePreRef;
	}

	string toString()
	{
		stringstream temp;
		temp << left << setw(6) << setfill(' ') << this->symbol << " " << right << setw(5) << setfill('0') << uppercase << hex << address;

		return temp.str();
	}
};
list <SymbolTable> symtab;

class TextRecord
{
public:
	string startLoction;
	int size;
	string record;

	TextRecord() // 預設建構子
	{
		stringstream temp;

		temp << setw(6) << setfill('0') << uppercase << hex << locationCounter; // 做成0xXXXXXX的樣式
		startLoction = "T^" + temp.str() + "^";

		size = 0;

		record = "^";
	}

	TextRecord(int localcounter)
	{
		stringstream temp;

		temp << setw(6) << setfill('0') << uppercase << hex << localcounter; // 做成0xXXXXXX的樣式
		startLoction = "T^" + temp.str() + "^";

		size = 0;

		record = "^";
	}

	bool insertTextRecord(string theRecord, int theSize)
	{
		if (this->size + theSize > 30) // 一行Text Record的最大長度為 30(based on Dec) 1E (based on Hex)
		{
			return false;
		}
		else
		{
			this->record += theRecord + "^";
			this->size += theSize;

			return true;
		}
	}

	string toString()
	{
		if (this->size == 0)
		{
			return "";
		}
		else
		{
			stringstream temp;
			string StRec;

			temp << setw(2) << setfill('0') << uppercase << hex << this->size;
			StRec = this->startLoction + temp.str() + this->record.substr(0, this->record.length() - 1); // 把額外的"^"去除，並將起始位置.大小.內容串接在一起

			return StRec;
		}
	}
};
list<TextRecord> textRecord;

class AsmResult
{
public:
	int loc;
	int flag;
	string label;
	string opcode;
	string oprand;
	string tempDot;

	AsmResult(int theLoc, string theLabel, string theOpcode, string theOprand)
	{
		this->loc = theLoc;
		this->flag = 0;
		this->label = theLabel;
		this->opcode = theOpcode;
		this->oprand = theOprand;
		this->tempDot = "";
	}
	AsmResult(string theTemp)
	{
		this->loc = -1;
		this->flag = 1;
		this->label = "";
		this->opcode = "";
		this->oprand = "";
		this->tempDot = theTemp;
	}

	string toString()
	{
		stringstream temp;
		if (flag == 0)
		{
			if (this->opcode == "END")
			{
				temp << setw(7) << setfill(' ') << left << setw(6) << setfill(' ') << this->label << " " << setw(6) << this->opcode << " " << setw(8) << this->oprand;
			}
			else
			{

				temp << setw(5) << setfill('0') << uppercase << hex << this->loc << ": " << left << setw(6) << setfill(' ') << this->label << " " << setw(6) << this->opcode << " " << setw(8) << this->oprand;
			}
		}
		else
		{
			temp << "       " << tempDot;
		}

		return temp.str();
	}
};
list<AsmResult> lst;

int insertToSymbolTable(string, int, TextRecord&);
void insertPreReference(string, int);
int findSymbolValue(string);
int SprateString(string, vector<string>&);
string findOpcode(string);
int checkX(string);
void printObj(string, string);
void printLst();
void printStb();

int main(int argc, char const* argv[])
{
	input.open(string(argv[1]) + ".asm", ios::in);  // Filename : argv[1]     To c++ string : string(argv[1])
	//input.open("test.asm", ios::in);
	optab.open("optab.txt", ios::in);

	string temp, HRecord, ERecord, label, opcode, oprand;
	int length, flag, allsize;

	getline(input, temp);
	Expline++;
	flag = SprateString(temp, buffer);

	if (buffer[0] == "END")
	{
		label = "";
		opcode = "END";
		oprand = buffer[1];
		AsmResult AsmTemp(locationCounter, label, opcode, oprand);
		lst.push_back(AsmTemp);
	}
	else if (buffer[0] == ".")
	{
		label = ".";
		opcode = "";
		oprand = "";
		AsmResult AsmTemp(temp);
		lst.push_back(AsmTemp);
	}
	else
	{
		if (flag == 3)
		{
			label = buffer[0];
			opcode = buffer[1];
			oprand = buffer[2];
		}
		else if (flag == 2)
		{
			if (findOpcode(buffer[0]) != "null")
			{
				label = "";
				opcode = buffer[0];
				oprand = buffer[1];
			}
			else
			{
				label = buffer[0];
				opcode = buffer[1];
				oprand = "";
			}
		}
		else
		{
			label = "";
			opcode = buffer[0];
			oprand = "";
		}
	}
	buffer.clear();

	if (opcode == "START")
	{
		stringstream tempR, tempL;

		tempR << setfill('0') << uppercase << hex << oprand;
		tempR >> locationCounter;
		tempL << setw(6) << setfill(' ') << left << label;

		startAddr = locationCounter;
		HRecord = "H^" + tempL.str() + tempR.str() + "^";

		AsmResult AsmTemp(locationCounter, label, opcode, oprand);
		lst.push_back(AsmTemp);
		insertToSymbolTable(label, locationCounter, *new TextRecord());

		getline(input, temp);
		Expline++;
		flag = SprateString(temp, buffer);

		if (buffer[0] == "END")
		{
			label = "";
			opcode = "END";
			oprand = buffer[1];
			AsmResult AsmTemp(locationCounter, label, opcode, oprand);
			lst.push_back(AsmTemp);
		}
		else if (buffer[0] == ".")
		{
			label = ".";
			opcode = "";
			oprand = "";
			AsmResult AsmTemp(temp);
			lst.push_back(AsmTemp);
		}
		else
		{
			if (flag == 3)
			{
				label = buffer[0];
				opcode = buffer[1];
				oprand = buffer[2];
			}
			else if (flag == 2)
			{
				if (findOpcode(buffer[0]) != "null")
				{
					label = "";
					opcode = buffer[0];
					oprand = buffer[1];
				}
				else
				{
					label = buffer[0];
					opcode = buffer[1];
					oprand = "";
				}
			}
			else
			{
				label = "";
				opcode = buffer[0];
				oprand = "";
			}
		}
	}
	else
	{
		cout << "There is no Start in the first line" << endl;
		system("pause");
		exit(1);
	}
	buffer.clear();

	TextRecord progressTextRecord;

	while (getline(input, temp))
	{
		if (label[0] != '.')
		{
			length = 0;
			string record = "";

			AsmResult AsmTemp(locationCounter, label, opcode, oprand);
			lst.push_back(AsmTemp);

			if (label != "")
			{
				insertToSymbolTable(label, locationCounter, progressTextRecord);
			}

			if (findOpcode(opcode) != "null")
			{
				length = 3;
				record += findOpcode(opcode);

				if (oprand != "")
				{
					int indexDec = checkX(oprand);

					if (indexDec > 0)
					{
						for (int j = 0; j < oprand.length(); j++)
						{
							if (oprand.at(j) == ',')
							{
								oprand = oprand.substr(0, j);
							}
						}
					}
					int sv = findSymbolValue(oprand);
					stringstream tempR;

					if (sv == -1)
					{
						insertPreReference(oprand, locationCounter + 1);
						tempR << setw(4) << setfill('0') << uppercase << hex << indexDec;
						record += tempR.str();
					}
					else
					{
						tempR << setw(4) << setfill('0') << uppercase << hex << (sv + indexDec);
						record += tempR.str();
					}
				}
				else
				{
					record += "0000";
				}
			}
			else if (opcode == "BYTE")
			{
				int trulen = oprand.length() - 3;
				if (oprand.at(0) == 'X')
				{
					record = oprand.substr(2, trulen);
					trulen /= 2;
				}
				else
				{
					string tempC = oprand.substr(2, trulen);

					for (int i = 0; i < tempC.length(); i++)
					{
						int ascii = tempC.at(i);

						stringstream tempR;
						tempR << uppercase << hex << ascii;

						record += tempR.str();
					}
				}
				length = trulen;
			}
			else if (opcode == "WORD")
			{
				stringstream tempR;
				tempR << setw(6) << setfill('0') << uppercase << hex << atoi(oprand.c_str());

				record = tempR.str();
				length = 3;
			}
			else if (opcode == "RESW")
			{
				locationCounter += 3 * atoi(oprand.c_str());
				length = -1;
			}
			else if (opcode == "RESB")
			{
				locationCounter += atoi(oprand.c_str());
				length = -1;
			}
			else
			{

				cout << "There has a expectation in line " << dec << Expline; //錯誤偵測，以便於Debug
				cout << "Because the opcode is not on optable." << endl;
				return 0;
			}

			if (length == -1)
			{
				textRecord.push_back(progressTextRecord);
				progressTextRecord = *new TextRecord();
			}
			else
			{
				locationCounter += length;

				if (!progressTextRecord.insertTextRecord(record, length))
				{
					textRecord.push_back(progressTextRecord);
					progressTextRecord = *new TextRecord();
					progressTextRecord.insertTextRecord(record, length);
				}
			}
		}

		Expline++;
		flag = SprateString(temp, buffer);

		if (buffer[0] == "END")
		{
			label = "";
			opcode = "END";
			oprand = buffer[1];
			AsmResult AsmTemp(locationCounter, label, opcode, oprand);
			lst.push_back(AsmTemp);
		}
		else if (buffer[0] == ".")
		{
			label = ".";
			opcode = "";
			oprand = "";
			AsmResult AsmTemp(temp);
			lst.push_back(AsmTemp);
		}
		else
		{
			if (flag == 3)
			{
				label = buffer[0];
				opcode = buffer[1];
				oprand = buffer[2];
			}
			else if (flag == 2)
			{
				if (findOpcode(buffer[0]) != "null")
				{
					label = "";
					opcode = buffer[0];
					oprand = buffer[1];
				}
				else
				{
					label = buffer[0];
					opcode = buffer[1];
					oprand = "";
				}
			}
			else
			{
				label = "";
				opcode = buffer[0];
				oprand = "";
			}
		}
		buffer.clear();
	}

	textRecord.push_back(progressTextRecord);

	allsize = locationCounter - startAddr;

	stringstream HCrecord;

	HCrecord << setw(6) << setfill('0') << uppercase << hex << allsize;
	HRecord += HCrecord.str();

	stringstream ECRecord;

	ECRecord << setw(6) << setfill('0') << uppercase << hex << findSymbolValue(oprand);
	ERecord = "E^" + ECRecord.str();

	string type;

	if (string(argv[1]) == "test")
	{
		outputR.open("test.obj", ios::out);
		printObj(HRecord, ERecord);

		outputR.close();

		if (string(argv[2]) == "-s")
		{
			outputL.open("test.lst", ios::out);

			printLst();

			outputL.close();
		}
		else if (string(argv[2]) == "-t")
		{
			outputS.open("test.stb", ios::out);
			printStb();

			outputS.close();
		}
		else if (string(argv[2]) == "-a")
		{
			outputL.open("test.lst", ios::out);
			outputS.open("test.stb", ios::out);
			printLst();
			printStb();

			outputL.close();
			outputS.close();
		}
		else
		{
			cout << "Wrong parameter !!, it only output obj file !!" << endl;
			return 0;
		}
		cout << "Commend executed successfully !!" << endl;
	}

	input.close();
	optab.close();

	return 0;
}

int insertToSymbolTable(string theLabel, int theLocctr, TextRecord& progressTextRecord)
{
	list<SymbolTable>::iterator i;

	for (i = symtab.begin(); i != symtab.end(); ++i)
	{
		if (i->symbol == theLabel)
		{
			if (i->address == -1)
			{
				i->address = theLocctr;
				if (progressTextRecord.size > 0)
				{
					textRecord.push_back(progressTextRecord);
					progressTextRecord = *new TextRecord();
				}

				list<int>::iterator preAddr;
				stringstream temp;

				temp << setw(4) << setfill('0') << uppercase << hex << theLocctr;

				for (preAddr = i->preRef.begin(); preAddr != i->preRef.end(); preAddr++)
				{
					TextRecord preReferenceTextRecord = *new TextRecord(*preAddr);
					preReferenceTextRecord.insertTextRecord(temp.str(), 2);
					textRecord.push_back(preReferenceTextRecord);
				}
				return 1;
			}
			else
			{
				cout << "There has a expectation in line " << dec << Expline; //錯誤偵測，以便於Debug
				return 0;
			}
		}
	}
	symtab.push_back(*new SymbolTable(theLabel, theLocctr));
	return 1;
}

void insertPreReference(string theLabel, int theLocctr)
{
	list<SymbolTable>::iterator i;

	for (i = symtab.begin(); i != symtab.end(); ++i)
	{
		if (i->symbol == theLabel)
		{
			i->preRef.push_back(theLocctr);
			return;
		}
	}

	list<int> valueLink;
	valueLink.push_back(theLocctr);
	symtab.push_back(*new SymbolTable(theLabel, valueLink));
}

int findSymbolValue(string theLabel)
{
	list<SymbolTable>::iterator i;

	for (i = symtab.begin(); i != symtab.end(); ++i)
	{
		if (i->symbol == theLabel)
		{
			return i->address;
		}
	}
	return -1;
}

int SprateString(string theString, vector<string>& buffer) //改過
{
	int current = 0; //最初由 0 的位置開始找
	int next, flag = 0;
	while (1)
	{
		next = theString.find_first_of(" \t", current);
		if (next != current)
		{
			string temp = theString.substr(current, next - current);
			if (temp.size() != 0)
			{//忽略空字串
				buffer.push_back(temp);
				flag++;
			}
		}
		if (next == string::npos)
		{
			break;
		}
		current = next + 1; //下次由 next + 1 的位置開始找起。
	}
	return flag;
}

string findOpcode(string theOpcode)
{
	optab.clear(); // 清除前次讀檔指針
	optab.seekg(0, ios::beg); // 將讀檔的指針設在檔首，偏移量為0

	string temp, tempValue, tempOpcode;

	while (getline(optab, temp))
	{
		SprateString(temp, bufferop);
		tempOpcode = bufferop[0];
		tempValue = bufferop[1];

		if (theOpcode == tempOpcode)
		{
			return tempValue;
		}
		bufferop.clear();
	}
	return "null";
}

int checkX(string theOprand)
{
	for (int i = 0; i < theOprand.length(); i++)
	{
		if (theOprand.at(i) == ',')
		{
			if (theOprand.at(i + 1) == 'X')
			{
				return 32768;
			}
		}
	}
	return 0;
}

void printObj(string HRC, string ERC)
{
	outputR << HRC << endl;

	list<TextRecord>::iterator i;

	for (i = textRecord.begin(); i != textRecord.end(); i++)
	{
		if ((*i).toString() != "")
		{
			outputR << (*i).toString() << endl;
		}
	}
	outputR << ERC;
}

void printLst()
{
	outputL << "Loc.   Source statement" << endl;
	outputL << "=====  ==================================" << endl;

	list<AsmResult>::iterator j;

	for (j = lst.begin(); j != lst.end(); j++)
	{
		if ((*j).toString() != "")
		{
			outputL << (*j).toString() << endl;
		}
	}
}

void printStb()
{
	list<SymbolTable>::iterator k;

	outputS << "Symbol Value" << endl;
	outputS << "====== ======" << endl;

	for (k = symtab.begin(); k != symtab.end(); k++)
	{
		if ((*k).toString() != "")
		{
			outputS << (*k).toString() << endl;
		}
	}
}