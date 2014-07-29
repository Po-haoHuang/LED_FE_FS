FeatureExtraction:

1.gsl 1.15 32bit  https://code.google.com/p/oscats/downloads/list
	說明：解壓放C根目錄  Compile 時 linker 下指令連結此函式庫
	EX:  ../../../../GSL-1.15/lib/libgsl.a ../../../../GSL-1.15/lib/libgslcblas.a

2.QT 設計gui
	32bit 5.3 mingw 4.8.2 
	default dll linking
	如果要standalone的exe,需built static qt
	附上 built 好的static版 如無法使用需重built 參考以下
	http://qt-project.org/wiki/How-to-build-a-static-Qt-for-Windows-MinGW

3.DataBase:
	如需使用快速讀取功能(csv.h),compile時加以下指令
	-std=c++11 -D__NO_MINGW_LFS -DUSE_FAST_CSV 
	Fast CSV can only read fixed CSV format (i.e. exactly 8 columns 	containing "DataTime", "dP_Filter (X1)", ...)


FeatureSelecyion:

Regression based:

mlpack(使用mingw make)
1.libxml libconv
	下載位置(直接使用prebuilt的只能使用.dll.a版本)
	ftp://ftp.zlatkovic.com/pub/libxml/64bit/
	或是自己重編static版(我是自己編)
	http://stackoverflow.com/questions/3429101/building-the-latest-iconv-and-libxml2-binaries-in-win32
	linker指令(如果需要的話)
	..\libxml2-2.9.1-win32-x86\lib\libxml2.dll.a ..\libxml2-2.9.1-win32-x86\lib\libxml2.a  ..\libiconv-1.8-20020830\lib\libiconv.a ..\libiconv-1.8-20020830\lib\libcharset.a


2.armadillo-4.320.0(enable lapack/blas)
	如何enable lapack/blas：
	include\armadillo_bits\config.hpp
	#define ARMA_USE_LAPACK
	#define ARMA_USE_BLAS
	這兩行uncomment
	linker指令
	..\Lapack_win32_release\blas_win32_MT.lib ..\Lapack_win32_release\lapack_win32_MT.lib
	並把兩dll放到執行檔旁

3.何處下載lapack/blas
	http://ylzhao.blogspot.tw/2013/10/blas-lapack-precompiled-binaries-for.html

4.boost
	可以自己編或使用含boost的mingw
	含boost mingw :http://nuwen.net/mingw.html

	自己編:
	http://stackoverflow.com/questions/20265879/how-to-build-boost-1-55-with-mingw
	http://www.boost.org/


4.mlpack本身(很難編...)
	參考
	http://www.mlpack.org/trac/wiki/MLPACKOnWindows
	所需套件下載完後(lapack 64bit,libxml2 64bit)
	注意Cmakelists裡面comment掉下面這兩段段再用cmake:
	#if (WIN32)
	#  link_directories(${Boost_LIBRARY_DIRS})
	#  set(Boost_LIBRARIES "")
	#endif (WIN32)

	#add_definitions(-DBOOST_TEST_DYN_LINK)

	cmake裡可加上下列兩個entry
	Boost_NO_BOOST_CMAKE
	Boost_USE_STATIC_LIBS

	另外，如果compile之後錯在tree_test.cpp
	comment 掉log::的部分(共有兩行)

MI based:

FEAST:
	FEAST資料夾內
	Makefile做下列更改
	libFSToolbox.so : $(objects)
		$(LINKER) -L$(MITOOLBOXPATH) libMIToolbox.so -lm -shared -o libFSToolbox.so $(objects)

	先compile MIToolbox
	再compile FEAST