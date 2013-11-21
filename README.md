base64_filter
=============

base64 encoder and decoder for boost::iostreams.


usage
=============

As follows, please push to boost::iostreams::filtering_stream. 

boost::iostreams::filtering_istream	is;
is.push(USBoost::iostreams::base64_encoder());
is.push(boost::iostreams::file_source("test.txt",std::ios::binary));

boost::iostreams::filtering_ostream	os;
os.push(boost::iostreams::file_sink("result.txt",std::ios::binary));

boost::iostreams::copy(is,os);
