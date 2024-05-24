///@file	base64_filter.h
///	Base64フィルタ・ヘッダファイル

//※boost版

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
///		Base64エンコーダー
///@author UeSyu
///@date   2013/11/19
//----------------------------------------------------------
class base64_encoder
{
	//encoderの状態
	enum _State
	{
		None,
		Read,	//入力
		Write,	//出力

	};

public:
	///----[ エンコードする ]----
	///@param	v	[in]	エンコードするデータ配列
	///@param	e	[in]	エンコードされた文字列を入れる変数
	static void Encode(std::vector<boost::uint8_t> &v,std::string &e)
	{
		typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::vector<boost::uint8_t>::const_iterator,6,8> > base64_text;
		e.assign(base64_text(v.begin()),base64_text(v.end()));

		int padding_count = (4-e.length()%4)%4;
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

	///----[ コンストラクタ ]----
	base64_encoder() :
		m_State(None)
	{
	}

	///----[ デストラクタ ]----
	virtual ~base64_encoder()
	{
	}

	///----[ 読み込み ]----
	template<typename Source>
	std::streamsize read(Source& src, char_type* s, std::streamsize n)
	{
		assert(m_State!=Write);
		if(m_State!=Read)
		{
		//読み込み開始
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

	///----[ 書き込み ]----
	template<typename Sink>
	std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
	{
		assert(m_State!=Read);

		m_State=Write;
		m_vSrc.insert(m_vSrc.end(), s, s + n);
		return	n;
	}

	///----[ 閉じる ]----
	template<typename Sink>
	void close(Sink &snk, std::ios_base::openmode mode)
	{
		if(mode==std::ios_base::in)
		{
		//入力ストリームの場合

		} else {
		//出力ストリームの場合
			size_t	srcSize=m_vSrc.size();

			//変換
			std::string	dst;
			Encode(m_vSrc,dst);
			boost::iostreams::write(snk,(const char*)dst.c_str(),dst.size());
		}
	}




protected:
	///----[ 読み込み開始 ]----
	template<typename Source>
	void BeginRead(Source& src)
	{
	//※展開フィルタと同じようにあらかじめ全部読み込んでおく

		//データを読み込む
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

		//変換
		Encode(vData,m_DstString);
		m_Count=0;
	}
protected:
//======[ メンバ変数 ]======
	_State	m_State;	///<状態

	std::streamsize	m_Count;	///<バッファから読み込んだバイト数

	std::string	m_DstString;	///<変換後の文字列
	std::vector<boost::uint8_t>	m_vSrc;	///<元データ受け取り用
};

//----------------------------------------------------------
///@class	base64_decoder
///		Base64デコーダー
///@author UeSyu
///@date   2013/11/20
//----------------------------------------------------------
class base64_decoder
{
	//encoderの状態
	enum _State
	{
		None,
		Read,	//入力
		Write,	//出力

	};
public:
	///----[ エンコードする ]----
	///@param	src	[in]	Base64エンコードされてる文字
	///@param	dst	[in]	デコードされたデータを入れる配列変数
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

	///----[ コンストラクタ ]----
	base64_decoder() :
		m_State(None)
	{
	}

	///----[ デストラクタ ]----
	virtual ~base64_decoder()
	{
	}


	//--------------------------------------
	//	インスタンスの操作

	///----[ 読み込み ]----
	template<typename Source>
	std::streamsize read(Source& src, char_type* s, std::streamsize n)
	{
		assert(m_State!=Write);
		if(m_State!=Read)
		{
		//読み込み開始
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

	///----[ 書き込み ]----
	template<typename Sink>
	std::streamsize write(Sink& snk, const char_type* s, std::streamsize n)
	{
		assert(m_State!=Read);

		m_State=Write;
		m_SrcString.insert(m_SrcString.end(), s, s + n);
		return	n;
	}

	///----[ 閉じる ]----
	template<typename Sink>
	void close(Sink &snk, std::ios_base::openmode mode)
	{
		if(mode==std::ios_base::in)
		{
		//入力ストリームの場合

		} else {
		//出力ストリームの場合
			size_t	srcSize=m_SrcString.size();

			//変換
			Decode(m_SrcString,m_vBuffer);

			boost::iostreams::write(snk,(const char*)&m_vBuffer[0],m_vBuffer.size());
		}
	}

protected:
	///----[ 読み込み開始 ]----
	template<typename Source>
	void BeginRead(Source& src)
	{
	//※展開フィルタと同じようにあらかじめ全部読み込んでおく

		//データを読み込む
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
//======[ メンバ変数 ]======
	_State	m_State;	///<状態

	std::streamsize	m_Count;	///<バッファから読み込んだバイト数

	std::vector<boost::uint8_t>	m_vBuffer;	///<変換後のデータを入れておくバッファ
	std::streamsize	m_BufferSize;	///<変換後のデータを入れておくバッファのサイズ

	std::string	m_SrcString;	///<変換元の文字列保存用バッファ

};

};	//end namespace iostreams

};	//end namespace USBoost

#endif
