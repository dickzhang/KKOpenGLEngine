#include "TileDataFile.h"

TileDataFile::TileDataFile(const std::string& filename, VirtualTextureInfo _info, bool _readWrite) : m_info(_info)
{
	const char* access = _readWrite ? "w+b" : "rb";
	m_file = fopen(filename.c_str(), access);
	m_size = m_info.GetPageSize() * m_info.GetPageSize() * s_channelCount;
}

TileDataFile::~TileDataFile()
{
	fclose(m_file);
}

void TileDataFile::readInfo()
{
	fseek(m_file, 0, SEEK_SET);
	auto ret = fread(&m_info, sizeof(m_info), 1, m_file);
	m_size = m_info.GetPageSize() * m_info.GetPageSize() * s_channelCount;
}

void TileDataFile::writeInfo()
{
	fseek(m_file, 0, SEEK_SET);
	auto ret = fwrite(&m_info, sizeof(m_info), 1, m_file);
}

void TileDataFile::readPage(int index, uint8_t* data)
{
	fseek(m_file, m_size * index + s_tileFileDataOffset, SEEK_SET);
	auto ret = fread(data, m_size, 1, m_file);
}

void TileDataFile::writePage(int index, uint8_t* data)
{
	fseek(m_file, m_size * index + s_tileFileDataOffset, SEEK_SET);
	auto ret = fwrite(data, m_size, 1, m_file);
}