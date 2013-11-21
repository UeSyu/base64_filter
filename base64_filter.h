///@file	base64_filter.h
///	Base64�t�B���^�E�w�b�_�t�@�C��

//��boost��

#ifndef __base64_filter_H__
#define __base64_filter_H__

#include <boost/cstdint.hpp>

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/operations.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace USBoost
{

namespace iostreams
{
//----------------------------------------------------------
///@class	base64_encoder
///		Base64�G���R�[�_�[
///@author UeSyu
///@date   2013/11/19
//----------------------------------------------------------
class base64_encoder
{
	//encoder�̏��
	enum _State
	{
		None,
		Read,	//����
		Write,	//�o��

	};

public:
	///----[ �G���R�[�h���� ]----
	///@param	v	[in]	�G���R�[�h����f�[�^�z��
	///@param	e	[in]	�G���R�[�h���ꂽ�����������ϐ�
	static void Encode(std::vector<boost::uint8_t> &v,std::string &e)
	{
		typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::vector<boost::uint8_t>::const_iterator,6,8> > base64_text;
		e.assign(base64_text(v.begin()),base64_text(v.end()));

		int padding_count = 4-e.length()%4;
		for (int i=0; i<padding_count; i++)
		{
			e += '=';
		}
	}

	typedef char	char_type;

	struct category :
			boost::iostreams::dual_use,
			boost::iostreams::filter_tag,
			boost::iostreams::multichar_tag,
			boost::iostreams::closable_tag
	{};

	///----[ �R���X�g���N�^ ]----
	base64_encoder() :
		m_State(None)
	{
	}

	///----[ �f�X�g���N�^ ]----
	virtual ~base64_encoder()
	{
	}

	///----[ �ǂݍ��� ]----
	template<typename Source>
	std::streamsize read(Source& src, char_type* s, std::streamsize n)
	{
		assert(m_State!=Write);
		if(m_State!=Read)
		{
		//�ǂݍ��݊J�n
			BeginRead(src);
			m_State=Read;
		}
		std::streamsize	readSize=(std::min)(n,static_cast<std::streamsize>(m_DstString.size()-m_Count));
		if(readSize>0)
		{
			memcpy(s,&m_DstString[m_Count],readSize);
			m_Count+=readSize;
			return	readSize;
		}

		return	-1;
	}

	///----[ �������� ]----
	template<typename Sink>
	std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
	{
		assert(m_State!=Read);

		m_State=Write;
		m_vSrc.insert(m_vSrc.end(), s, s + n);
		return	n;
	}

	///----[ ���� ]----
	template<typename Sink>
	void close(Sink &snk, std::ios_base::openmode mode)
	{
		if(mode==std::ios_base::in)
		{
		//���̓X�g���[���̏ꍇ

		} else {
		//�o�̓X�g���[���̏ꍇ
			size_t	srcSize=m_vSrc.size();

			//�ϊ�
			std::string	dst;
			Encode(m_vSrc,dst);
			boost::iostreams::write(snk,(const char*)dst.c_str(),dst.size());
		}
	}




protected:
	///----[ �ǂݍ��݊J�n ]----
	template<typename Source>
	void BeginRead(Source& src)
	{
	//���W�J�t�B���^�Ɠ����悤�ɂ��炩���ߑS���ǂݍ���ł���

		//�f�[�^��ǂݍ���
		std::vector<boost::uint8_t>	vData;
		const std::streamsize  size = boost::iostreams::default_device_buffer_size;
		char	vBuffer[size];
		while(true)
		{
			std::streamsize	length=boost::iostreams::read(src, vBuffer, size);
			if(length==-1)
			{
				break;
			}

			vData.insert(vData.end(), vBuffer, vBuffer + length);
		}
		size_t	srcSize=vData.size();

		//�ϊ�
		Encode(vData,m_DstString);
		m_Count=0;
	}
protected:
//======[ �����o�ϐ� ]======
	_State	m_State;	///<���

	std::streamsize	m_Count;	///<�o�b�t�@����ǂݍ��񂾃o�C�g��

	std::string	m_DstString;	///<�ϊ���̕�����
	std::vector<boost::uint8_t>	m_vSrc;	///<���f�[�^�󂯎��p
};

//----------------------------------------------------------
///@class	base64_decoder
///		Base64�f�R�[�_�[
///@author UeSyu
///@date   2013/11/20
//----------------------------------------------------------
class base64_decoder
{
	//encoder�̏��
	enum _State
	{
		None,
		Read,	//����
		Write,	//�o��

	};
public:
	///----[ �G���R�[�h���� ]----
	///@param	src	[in]	Base64�G���R�[�h����Ă镶��
	///@param	dst	[in]	�f�R�[�h���ꂽ�f�[�^������z��ϐ�
	static void Decode(std::string &src,std::vector<boost::uint8_t> &dst)
	{
		typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6,boost::uint8_t> base64_binary;
		std::string::const_iterator	itBegin=src.begin(),itEnd=src.end();
		--itEnd;
		while((*itEnd)=='=')
		{
			--itEnd;
		}
		++itEnd;
		dst.assign(base64_binary(itBegin),base64_binary(itEnd));
	}

	typedef char	char_type;

	struct category :
			boost::iostreams::dual_use,
			boost::iostreams::filter_tag,
			boost::iostreams::multichar_tag,
			boost::iostreams::closable_tag
	{};

	///----[ �R���X�g���N�^ ]----
	base64_decoder() :
		m_State(None)
	{
	}

	///----[ �f�X�g���N�^ ]----
	virtual ~base64_decoder()
	{
	}


	//--------------------------------------
	//	�C���X�^���X�̑���

	///----[ �ǂݍ��� ]----
	template<typename Source>
	std::streamsize read(Source& src, char_type* s, std::streamsize n)
	{
		assert(m_State!=Write);
		if(m_State!=Read)
		{
		//�ǂݍ��݊J�n
			BeginRead(src);
			m_State=Read;
		}
		std::streamsize	readSize=(std::min)(n,static_cast<std::streamsize>(m_BufferSize-m_Count));
		if(readSize>0)
		{
			memcpy(s,&m_SrcString[m_Count],readSize);
			m_Count+=readSize;
			return	readSize;
		}

		return	-1;
	}

	///----[ �������� ]----
	template<typename Sink>
	std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
	{
		assert(m_State!=Read);

		m_State=Write;
		m_SrcString.insert(m_SrcString.end(), s, s + n);
		return	n;
	}

	///----[ ���� ]----
	template<typename Sink>
	void close(Sink &snk, std::ios_base::openmode mode)
	{
		if(mode==std::ios_base::in)
		{
		//���̓X�g���[���̏ꍇ

		} else {
		//�o�̓X�g���[���̏ꍇ
			size_t	srcSize=m_SrcString.size();

			//�ϊ�
			Decode(m_SrcString,m_vBuffer);

			boost::iostreams::write(snk,(const char*)&m_vBuffer[0],m_vBuffer.size());
		}
	}

protected:
	///----[ �ǂݍ��݊J�n ]----
	template<typename Source>
	void BeginRead(Source& src)
	{
	//���W�J�t�B���^�Ɠ����悤�ɂ��炩���ߑS���ǂݍ���ł���

		//�f�[�^��ǂݍ���
		std::string	vData;
		const std::streamsize  size = boost::iostreams::default_device_buffer_size;
		char	vBuffer[size];
		while(true)
		{
			std::streamsize	length=boost::iostreams::read(src, vBuffer, size);
			if(length==-1)
			{
				break;
			}

			vData.insert(vData.end(), vBuffer, vBuffer + length);
		}
		size_t	srcSize=vData.size();
		Decode(vData,m_vBuffer);
		m_BufferSize=m_vBuffer.size();
		m_Count=0;
	}
protected:
//======[ �����o�ϐ� ]======
	_State	m_State;	///<���

	std::streamsize	m_Count;	///<�o�b�t�@����ǂݍ��񂾃o�C�g��

	std::vector<boost::uint8_t>	m_vBuffer;	///<�ϊ���̃f�[�^�����Ă����o�b�t�@
	std::streamsize	m_BufferSize;	///<�ϊ���̃f�[�^�����Ă����o�b�t�@�̃T�C�Y

	std::string	m_SrcString;	///<�ϊ����̕�����ۑ��p�o�b�t�@

};

};	//end namespace iostreams

};	//end namespace USBoost

#endif