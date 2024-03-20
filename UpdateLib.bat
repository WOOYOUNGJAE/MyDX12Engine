

//// xcopy	.복사할 파일이 들어있는 경로			.복사받을 경로.



xcopy	/y .\Engine\Bin\Intermediate\CompiledShaders\*.*					.\Engine\Public\Shaders
xcopy	/y/s .\Engine\Public\*.*							.\Reference\Headers\
xcopy	/y .\Engine\Bin\*.lib							.\Reference\Libraries\
xcopy	/y .\Engine\Bin\*.dll							.\Client\Bin\
xcopy	/y .\Engine\Bin\Shader\*.*					.\Client\Bin\Shader\