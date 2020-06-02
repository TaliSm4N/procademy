#include <iostream>
#include <Windows.h>
#include "HTTP.h"

int main()
{
	CHTTP http;
	char test[1500];

	if (http.SetURL(L"http://127.0.0.1"))
	{
		printf("success\n");
	}
	
	if (http.HTTPPost(L"auth_reg.php", "{\"id\":\"fuckdding\",\"password\":\"test\",\"nick\":\"uddvu\"}", test))
	{
		printf("ture\n\n");
		printf("%s",test);
	}
	else
	{
		printf("fuck");
	}


	system("pause");
}