================================================================================
0. Introduction
================================================================================
Feature Extraction & Feature Selection


================================================================================
1. Program Usage 
================================================================================

FE_no_GUI.exe dir(cyclelist.csv) dir(rowdata) Cyclebegin(num) Cycleend(num) segmentnum

dir(cyclelist.csv) ����۹���|�ҥi
dir(rowdata)	   ����۹���|�ҥi
Cyclebegin(num)    �_�lcycle number
Cycleend(num)      ����cycle number
segmentnum         ������

�@5�ӰѼ�
--------------------------------------------------------------------------------

FS_no_GUI.exe input_file target_feature disct_method fcbf_thrd ridge_lambda lasso_lambda els_lambda1 els_lambda2 print_n score_method top_k

1.  input_file				FE�B�z�᪺�ɮ�(Output_noSeg.csv, Output_seg1.csv, Output_seg2.csv)
2.  target_feature			�ؼЯS�x("dP_Filter (X1)_max")
3.  disct_method			���w�����Ƥ覡("ew_cycle=4", "5,15,20")
4.  fcbf_thrd				MI-FCBF�t��k��threshold (0.01)
5.  ridge_lambda			Regression-RIDGE�t��k��lambda(1, 2, 3)	
6.  lasso_lambda			Regression-LASSO�t��k��lambda(1, 2, 3)
7.  els_lambda1				Regression-ElasticNet�t��k��lambda1(1, 2, 3)
8.  els_lambda2				Regression-ElasticNet�t��k��lambda2(1, 2, 3)
9.  print_n					�C�L�en�W�����G(1~n)
10. score_method			�����覡(1)
11. top_k					�t��k�z�諸�Ӽ�(15,20,30 ...)

ex: Output_noSeg.csv "dP_Filter (X1)_max" "ew_cycle=4" 0.01 1 1 1 1 10 1 15

�@11�ӰѼ�


================================================================================
2. Build from Soucre Code
================================================================================

FeatureExtraction:

1.gsl 1.15 32bit  https://code.google.com/p/oscats/downloads/list
	�����G������C�ڥؿ�  Compile �� linker �U���O�s�����禡�w
	EX:  ../../../../GSL-1.15/lib/libgsl.a ../../../../GSL-1.15/lib/libgslcblas.a

2.QT �]�pgui
	32bit 5.3 mingw 4.8.2 
	default dll linking
	�p�G�nstandalone��exe,��built static qt
	���W built �n��static�� �p�L�k�ϥλݭ�built �ѦҥH�U
	http://qt-project.org/wiki/How-to-build-a-static-Qt-for-Windows-MinGW

3.DataBase:
	�p�ݨϥΧֳtŪ���\��(csv.h),compile�ɥ[�H�U���O
	-std=c++11 -D__NO_MINGW_LFS -DUSE_FAST_CSV 
	Fast CSV can only read fixed CSV format (i.e. exactly 8 columns 	containing "DataTime", "dP_Filter (X1)", ...)


FeatureSelecyion:

Regression based:

mlpack(�ϥ�mingw make)
1.libxml libconv
	�U����m(�����ϥ�prebuilt���u��ϥ�.dll.a����)
	ftp://ftp.zlatkovic.com/pub/libxml/64bit/
	�άO�ۤv���sstatic��(�ڬO�ۤv�s)
	http://stackoverflow.com/questions/3429101/building-the-latest-iconv-and-libxml2-binaries-in-win32
	linker���O(�p�G�ݭn����)
	..\libxml2-2.9.1-win32-x86\lib\libxml2.dll.a ..\libxml2-2.9.1-win32-x86\lib\libxml2.a  ..\libiconv-1.8-20020830\lib\libiconv.a ..\libiconv-1.8-20020830\lib\libcharset.a


2.armadillo-4.320.0(enable lapack/blas)
	�p��enable lapack/blas�G
	include\armadillo_bits\config.hpp
	#define ARMA_USE_LAPACK
	#define ARMA_USE_BLAS
	�o���uncomment
	linker���O
	..\Lapack_win32_release\blas_win32_MT.lib ..\Lapack_win32_release\lapack_win32_MT.lib
	�ç��dll�������ɮ�

3.��B�U��lapack/blas
	http://ylzhao.blogspot.tw/2013/10/blas-lapack-precompiled-binaries-for.html

4.boost
	�i�H�ۤv�s�ΨϥΧtboost��mingw
	�tboost mingw :http://nuwen.net/mingw.html

	�ۤv�s:
	http://stackoverflow.com/questions/20265879/how-to-build-boost-1-55-with-mingw
	http://www.boost.org/


4.mlpack����(�����s...)
	�Ѧ�
	http://www.mlpack.org/trac/wiki/MLPACKOnWindows
	�һݮM��U������(lapack 64bit,libxml2 64bit)
	�`�NCmakelists�̭�comment���U���o��q�q�A��cmake:
	#if (WIN32)
	#  link_directories(${Boost_LIBRARY_DIRS})
	#  set(Boost_LIBRARIES "")
	#endif (WIN32)

	#add_definitions(-DBOOST_TEST_DYN_LINK)

	cmake�̥i�[�W�U�C���entry
	Boost_NO_BOOST_CMAKE
	Boost_USE_STATIC_LIBS

	�t�~�A�p�Gcompile������btree_test.cpp
	comment ��log::������(�@�����)

MI based:

FEAST:
	FEAST��Ƨ���
	Makefile���U�C���
	libFSToolbox.so : $(objects)
		$(LINKER) -L$(MITOOLBOXPATH) libMIToolbox.so -lm -shared -o libFSToolbox.so $(objects)

	��compile MIToolbox
	�Acompile FEAST