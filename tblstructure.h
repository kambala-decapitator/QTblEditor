#ifndef TBLSTRUCTURE_H
#define TBLSTRUCTURE_H

#include <QVector>
#include <QPair>
#include <QString>
#include <QObject>
#include <QDataStream>


// typedefs for understanding
typedef quint8  BYTE;
typedef quint16 WORD;
typedef quint32 DWORD;


#pragma pack(1)
struct TblHeader // header of string *.tbl file
{
	WORD  CRC;				  // +0x00 - CRC value for string table
	WORD  NodesNumber;		  // +0x02 - size of Indices array
	DWORD HashTableSize;	  // +0x04 - size of TblHashNode array
	BYTE  Version;			  // +0x08 - file version, either 0 or 1, doesn't matter
	DWORD DataStartOffset;	  // +0x09 - string table start offset
	DWORD HashMaxTries;		  // +0x0D - max number of collisions for string key search based on its hash value
	DWORD FileSize;			  // +0x11 - size of the file

	static const int size = 0x15;

	TblHeader() {}
	TblHeader(WORD crc, WORD nodesNumber, DWORD hashTableSize, BYTE version, DWORD dataStartOffset, DWORD hashMaxTries,
		DWORD fileSize) : CRC(crc), NodesNumber(nodesNumber), HashTableSize(hashTableSize), Version(version),
		DataStartOffset(dataStartOffset), HashMaxTries(hashMaxTries), FileSize(fileSize) {}
};

struct TblHashNode // node of the hash table in string *.tbl file
{
	BYTE  Active;			  // +0x00 - shows if the entry is used. if 0, then it has been "deleted" from the table
	WORD  Index;			  // +0x01 - index in Indices array
	DWORD HashValue;		  // +0x03 - hash value of the current string key
	DWORD StringKeyOffset;	  // +0x07 - offset of the current string key
	DWORD StringValOffset;	  // +0x0B - offset of the current string value
	WORD  StringValLength;	  // +0x0F - length of the current string value

	static const int size = 0x11;

	TblHashNode() {}
	TblHashNode(BYTE active, WORD index, DWORD hashValue, DWORD stringKeyOffset, DWORD stringValOffset, WORD stringValLength) :
		Active(active), Index(index), HashValue(hashValue), StringKeyOffset(stringKeyOffset),
		StringValOffset(stringValOffset), StringValLength(stringValLength) {}
};
#pragma pack()


struct DataNode // my node of the string table in string *.tbl file
{
	DWORD Index; // string key offset
	QString Key; // current string key
	QString Val; // current string value

	DataNode() {}
	DataNode(DWORD i, QString k, QString v) : Index(i), Key(k), Val(v) {}
};


QDataStream &operator >>(QDataStream &in, TblHeader &th);

class TblStructure : public QObject
{
public:
	TblStructure(QObject *parent = 0) : QObject(parent) {}

	TblHeader header() const { return _header; }
	QPair<QString, QString> dataStrings(WORD i) const { return QPair<QString, QString>(_data.at(i).Key, _data.at(i).Val); }

	void fillHeader(QDataStream &in) { in >> _header; }
	void getStringTable(QDataStream &in);

	static DWORD hashValue(char *key, int hashTableSize);
	static WORD getCRC(const char *stringData, DWORD size);

private:
	TblHeader _header;
	QVector<DataNode> _data;
};

#endif // TBLSTRUCTURE_H
