~~~c
namespace UniConvfunc{

//转换结构
struct IConvResult {
	std::string        result;           // 转换成功的结果 使用新编码的字符串
	int                error_code  = 0 ; // 错误码
	std::string        error_msg;        // 错误信息
};
IConvResult Convert(std::string_view in, char const * fromcode,char const* tocode) {
	IConvResult iconv_result;

	//打开iconv 转换描述符
	iconv_t cd = iconv_open(tocode, fromcode);
	if (cd == reinterpret_cast<iconv_t>(-1)) {
		iconv_result.error_code = errno;

		switch (iconv_result.error_code){
			case EINVAL:
				//不完整的多字节序列 or  不支持的编码
				iconv_result.error_msg = "Conversion not supported between the specified encodings";;
				break;
			case ENOMEM:
				// 内存不足
				iconv_result.error_msg = "Out of memory";
				break;
			default:
				iconv_result.error_msg = "Unknown error";
				break;
		}

		return iconv_result;
	}

	char*        inBufferPtr = const_cast<char*>(in.data());//指向输入缓存位置(非常量
	std::size_t  inBytesLeft = in.size();//输入缓存的大小

	

	std::stringstream ss;

	std::size_t const sizeOfOutbuffer = 4096;//固定大小的输出缓冲区大小
	char        outBuffer[sizeOfOutbuffer];//输出缓存


	while (inBytesLeft > 0 )// 输入缓存中,还有剩余字符未被转换
	{
		char*         outBufferPtr  = outBuffer;
		std::size_t   outBufferLeft = sizeOfOutbuffer;

		//执行转换 // 在iconv 中会修改buffer有多少个字节没有被使用
		std::size_t ret = iconv(cd, (const char**) & inBufferPtr, &inBytesLeft, &outBufferPtr, &outBufferLeft);

		if (ret == (std::size_t)(-1)) {
			// 转换停止 or 错误
			auto n = errno;
			switch (n)
			{
				case E2BIG:
				{
					break;
				}
				case EILSEQ:
				{
					//输入字符序列不符合编码规则
					iconv_result.error_msg = "Invalid multibyte sequence";
					iconv_result.error_code = errno;
					break;
				}
				case EINVAL:
				{
					//输入的字符序列不完整
					iconv_result.error_code = errno;
					iconv_result.error_msg = "Incomplete character sequence";
					break;
				}
				
				default:
				{
					iconv_result.error_code = errno;
					iconv_result.error_msg = "Unknown conversion error";
					break;
				}
				
			}
		}

		if (iconv_result.error_code != 0) {
			//跳出循环
			break;
		}
		
		//将本轮的输出结果，写入输出流
		ss.write(outBuffer, sizeOfOutbuffer - outBufferLeft);
	}

	iconv_close(cd);
	if (iconv_result.error_code == 0)
		iconv_result.result = ss.str();
	return iconv_result;
}
}

~~~

https://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.18/iconv_open.3.html
