#include "menu.h"
#include "game.h"

	/*
	Current menu
	0=press start
	1=main menu
	2=select player profile
	3=enter number of Players
	4=multiplayer 2
	5=multiplayer 3
	6=multiplayer 4
	7=code entry
	8=profiles editing (1 player)



	21=unlock area
	*/



//CMenu Menu;

/*
const GUID *ProfileIds[] = {
    &DXFILEOBJ_SimpleData,

    &DXFILEOBJ_ArrayData,
    &DXFILEOBJ_RestrictedData};
*/

PROFILE::PROFILE()
{
	ProfileName=NULL;
	ProfileColor=0;
	JoystickSensitivity.x=8.0f;
	JoystickSensitivity.y=8.0f;
	InvertYAxis=false;
	RelativeAiming=false;
	Stats.HighScore=0;
	CodesList[0]=NULL;

}

/*
int CMenu::ToLetters()
{
	if (m_SelectY>=0 && m_SelectY<=9)
	{
		return(m_SelectY);
	}
	else if (m_SelectY==36)
	{
		return(32);
	}
	else
	{
		return(m_SelectY+55);
	}
}

void CMenu::DrawProfileList()
{
	int NumTemp=m_SelectY;
	bool m_HasProfiles=false;
	char* Temp;
	for(std::list<PROFILE*>::iterator iProfile=m_ProfileList.begin();iProfile!=m_ProfileList.end();iProfile++)
	{	
		PROFILE* Profile=(*iProfile);
		Temp=Profile->ProfileName;
		m_HasProfiles=true;

		if (NumTemp==0)
		{
			break;
		}
		NumTemp--;
	}
	if (m_HasProfiles)
	{
		XEngine->DrawMenuText(Temp,80,250,255,120,100);
	}
	m_HasProfiles=false;
	for (int i=1; i<3; i++)
	{
		NumTemp=m_SelectY-i;
		if (!(NumTemp<0))
		{
			for(std::list<PROFILE*>::iterator iProfile=m_ProfileList.begin();iProfile!=m_ProfileList.end();iProfile++)
			{	
				PROFILE* Profile=(*iProfile);
				Temp=Profile->ProfileName;
				m_HasProfiles=true;

				if (NumTemp==0)
				{
					break;
				}
				NumTemp--;
			}
			if (m_HasProfiles)
			{
				m_HasProfiles=false;
				XEngine->DrawMenuText(Temp,80,250-(i*25),0,120,100);
			}
		}
	}
	m_HasProfiles=false;
	for (int i=1; i<3; i++)
	{
		NumTemp=m_SelectY+i;
		if (!(NumTemp>m_NumOfProfiles))
		{
			for(std::list<PROFILE*>::iterator iProfile=m_ProfileList.begin();iProfile!=m_ProfileList.end();iProfile++)
			{	
				PROFILE* Profile=(*iProfile);
				Temp=Profile->ProfileName;
				m_HasProfiles=true;

				if (NumTemp==0)
				{
					break;
				}
				NumTemp--;
			}
			if (m_HasProfiles)
			{
				XEngine->DrawMenuText(Temp,80,250+(i*25),0,120,100);
				m_HasProfiles=false;
			}
		}
	}

}

void CMenu::Init()
{

	m_SelectSound.Create("d:\\data\\Sounds\\Menu\\menuselect.wav");
	m_ChangeSound.Create("d:\\data\\Sounds\\Menu\\menuchange.wav");

	m_ProfileColor[0]=XEngine->LoadTexture("d:\\data\\Player\\Aqua\\Aqua_Stand.png");
	m_ProfileColor[1]=XEngine->LoadTexture("d:\\data\\Player\\Black\\Black_Stand.png");
	m_ProfileColor[2]=XEngine->LoadTexture("d:\\data\\Player\\Blue\\Blue_Stand.png");
	m_ProfileColor[3]=XEngine->LoadTexture("d:\\data\\Player\\Cyan\\Cyan_Stand.png");
	m_ProfileColor[4]=XEngine->LoadTexture("d:\\data\\Player\\Forest\\Forest_Stand.png");
	m_ProfileColor[5]=XEngine->LoadTexture("d:\\data\\Player\\Green\\Green_Stand.png");
	m_ProfileColor[6]=XEngine->LoadTexture("d:\\data\\Player\\Navy\\Navy_Stand.png");
	m_ProfileColor[7]=XEngine->LoadTexture("d:\\data\\Player\\Orange\\Orange_Stand.png");
	m_ProfileColor[8]=XEngine->LoadTexture("d:\\data\\Player\\Pink\\Pink_Stand.png");
	m_ProfileColor[9]=XEngine->LoadTexture("d:\\data\\Player\\Purple\\Purple_Stand.png");
	m_ProfileColor[10]=XEngine->LoadTexture("d:\\data\\Player\\Red\\Red_Stand.png");
	m_ProfileColor[11]=XEngine->LoadTexture("d:\\data\\Player\\White\\White_Stand.png");
	m_ProfileColor[12]=XEngine->LoadTexture("d:\\data\\Player\\Yellow\\Yellow_Stand.png");

	//m_ArrowRight=XEngine->LoadTexture("d:\\data\\menu\\Arrow_Right.png");

	m_BackGroundImage[0]=XEngine->LoadTexture("d:\\data\\Backgrounds\\BG4.png");
	
	m_SelectY=0;
	m_CurrentMenu=1;
	m_NumOfProfiles=0;

	XEngine->PlayMusic("d:\\data\\music\\title.wma");

	Game.m_GameSettings->Profiles[0]=new PROFILE;
	Game.m_GameSettings->Profiles[1]=new PROFILE;
	Game.m_GameSettings->Profiles[3]=new PROFILE;
	Game.m_GameSettings->Profiles[2]=new PROFILE;

	m_DispChars[0]=false;
	m_DispChars[1]=false;
	m_DispChars[2]=false;
	m_DispChars[3]=false;
	m_DispProf[0]=false;
	m_DispProf[1]=false;
	m_DispProf[2]=false;
	m_DispProf[3]=false;
	m_ReadyBegin[0]=true;
	m_ReadyBegin[1]=true;
	m_ReadyBegin[2]=true;
	m_ReadyBegin[3]=true;
	m_FirstRot=false;

	//char* m_FileName="D:\\data\\Data\\Data.bat";

	//m_GameHandle = CreateFile(m_FileName,GENERIC_ALL,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);


	//for(std::list<PROFILE*>::iterator iPROFILE=m_ProfileList.begin();iPROFILE!=m_ProfileList.end();iPROFILE++)
	//{	
	//	PROFILE* PROFILE=(*iPROFILE);
	//	m_NumOfProfiles++;
	//}

}


int CMenu::Update()
{
	/*
	if(XEngine->GamePad[0].wPressedAnalogButtons & XEngine->GamePad[0].fY1 > .3)
	{
		m_ChangeSound.PlaySound();
		
		Select++;
	}

	if(XEngine->GamePad[0].wPressedAnalogButtons & XEngine->GamePad[0].fY1 < -.3)
	{
		m_ChangeSound.PlaySound();

		Select--;
	}
*/
/*
	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{
		if ((XEngine->GamePad[m_GamePadButton].wPressedButtons & XINPUT_GAMEPAD_DPAD_LEFT) && m_CurrentMenu==2 && m_DispProf[m_GamePadButton])
		{
			switch (m_SelectY)
			case 1:
			{
				Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor--;
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor<0)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor=12;
				}
				break;
			case 2:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.x!=1)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.x-=1;
					Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.y-=1;
				}
				break;
			case 3:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->InvertYAxis==true)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->InvertYAxis=false;
				}
			case 4:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->RelativeAiming==true)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->RelativeAiming=false;
				}
			};
		}

		if ((XEngine->GamePad[m_GamePadButton].wPressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && m_CurrentMenu==2 && m_DispProf[m_GamePadButton])
		{
			switch (m_SelectY)
			{
			case 1:
				Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor++;
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor>12)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileColor=0;
				}
				break;
			case 2:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.x!=100)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.x+=1;
					Game.m_GameSettings->Profiles[m_GamePadButton]->JoystickSensitivity.y+=1;
				}
				break;
			case 3:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->InvertYAxis==false)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->InvertYAxis=true;
				}
			case 4:
				if (Game.m_GameSettings->Profiles[m_GamePadButton]->RelativeAiming==false)
				{
					Game.m_GameSettings->Profiles[m_GamePadButton]->RelativeAiming=true;
				}
			};
		}

		if (XEngine->GamePad[m_GamePadButton].wPressedButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
			m_ChangeSound.PlaySound();

			m_SelectY--;
		}

		if (XEngine->GamePad[m_GamePadButton].wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			m_ChangeSound.PlaySound();

			m_SelectY++;
		}
	}

	if (m_CurrentMenu==0)
	{
		m_SelectY=0;
	}
	if (m_CurrentMenu==1)
	{
		if (m_SelectY>3)
		{
			m_SelectY=0;
		}
		if (m_SelectY<0)
		{
			m_SelectY=3;
		}
	}

	if (m_CurrentMenu==2)
	{
		/*
		if (m_SelectY>m_NumOfProfiles)
		{
			m_SelectY=0;
		}
		if (m_SelectY<0)
		{
			m_SelectY=m_NumOfProfiles;
		}
		*//*
	}

	if (m_CurrentMenu==3)
	{
		if (m_SelectY>2)
		{
			m_SelectY=0;
		}
		if (m_SelectY<0)
		{
			m_SelectY=2;
		}
	}

	if (m_CurrentMenu==8)
	{
		if (m_SelectY>35)
		{
			m_SelectY=0;
		}
		if (m_SelectY<0)
		{
			m_SelectY=35;
		}
	}
	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{

		if(XEngine->GamePad[m_GamePadButton].bPressedAnalogButtons[1])
		{	
			if (m_DispChars[0])
			{
				m_DispChars[0]=false;
			}
			else
			{
				m_SelectSound.PlaySound();
				m_CurrentMenu=1;
				m_SelectY=0;
			}
		}
	}
	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{
		if(XEngine->GamePad[m_GamePadButton].bPressedAnalogButtons[2] && m_CurrentMenu==2 && m_ReadyBegin[m_GamePadButton]==true)
		{	
			Game.m_GameSettings->Profiles[m_GamePadButton]=new PROFILE;
			m_DispProf[m_GamePadButton]=true;
	
		}
	}

	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{

		if(XEngine->GamePad[m_GamePadButton].bPressedAnalogButtons[3] && m_CurrentMenu==2 && m_ReadyBegin==false)
		{	
			m_ReadyBegin[m_GamePadButton]=true;
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[0]='N';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[1]='o';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[2]=' ';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[3]='N';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[4]='a';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[5]='m';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[6]='e';
			Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName[7]=NULL;
		}
	}

	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{

		if(XEngine->GamePad[m_GamePadButton].wPressedButtons==XINPUT_GAMEPAD_START && m_CurrentMenu==2)
		{	
			//m_SelectSound.PlaySound();
			if (m_CurrentMenu==2 && m_DispChars[m_GamePadButton] && m_DispProf[m_GamePadButton]==false)
			{
				m_DispChars[m_GamePadButton]=false;
				m_DispProf[m_GamePadButton]=true;
			}
			if (Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName==NULL && m_DispProf[m_GamePadButton]==false)
			{
				m_NumberofPlayers++;
				m_ReadyBegin[m_GamePadButton]=false;
				m_DispProf[m_GamePadButton]=true;
			}
		}
	}

	//if statement for the Select Statement
	for(int m_GamePadButton=0; m_GamePadButton<3; m_GamePadButton++)
	{

		if(XEngine->GamePad[m_GamePadButton].bPressedAnalogButtons[0])
		{
			m_SelectSound.PlaySound();
			switch (m_SelectY)
			{
			case 0:
				

				if (m_CurrentMenu==1)
				{
					return 0;
				}
				break;

			case 1:
				if (m_CurrentMenu==1)
				{
					m_CurrentMenu=2;
					m_ReadyBegin[m_GamePadButton]=false;
					m_SelectY=0;
				}
				else if (m_CurrentMenu==2 && m_DispChars[m_GamePadButton]==false && m_DispProf[m_GamePadButton])
					{
						m_DispChars[m_GamePadButton]=true;
						m_DispProf[m_GamePadButton]=false;
						m_SelectY=1;
					}
				else if (m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,TempChar2);
				}
				break;

			case 2:
				if (m_CurrentMenu==1)
				{
					m_CurrentMenu=8;
					m_SelectY=0;
				}
				
				break;

			case 3:
				if(m_CurrentMenu==1)
				{
					XEngine->PauseMusic();
	
					XEngine->PlayVideo("d:\\data\\movies\\logo.wmv");
					XEngine->PlayVideo("d:\\data\\movies\\intro.wmv");
	
					XEngine->PauseMusic();
				}
				break;

						
			case 4:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{	
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 5:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 6:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 7:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 8:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 9:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 10:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 11:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 12:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 134:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 14:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 15:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 16:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 17:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 18:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 19:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;


			case 20:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 21:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 22:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 23:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 24:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 25:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 26:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 27:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 28:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 29:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 30:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 31:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 32:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 33:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 34:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 35:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			case 36:
				if(m_CurrentMenu==2 && m_DispChars[m_GamePadButton])
				{		
					char TempChar2[1]; TempChar2[0]=ToLetters();
					strcat(Game.m_GameSettings->Profiles[m_GamePadButton]->ProfileName,  TempChar2);
				}
				break;

			};
		}
	}
	Refresh();

	if(m_ReadyBegin[0] && m_ReadyBegin[1] && m_ReadyBegin[2] && m_ReadyBegin[3] && m_CurrentMenu==2) 
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

void CMenu::Refresh()
{
	int Red=0;
	int NumTemp=0;
	char* Temp="";
	bool m_HasProfiles=false;

	XEngine->DrawSprite(m_BackGroundImage[0],320,240,640,480,0);

	if (m_CurrentMenu==0)
	{
		XEngine->DrawMenuText("Press A", 200,200,200,200,200);
	}

	else if (m_CurrentMenu==1)
	{
		
		if (m_SelectY==MM_SINGLEPLAYER)
			Red=255;
		XEngine->DrawMenuText("Quick Start",80,250,Red,120,100);
		Red=0;

		if (m_SelectY==MM_MULTIPLAYER)
			Red=255;
		XEngine->DrawMenuText("Single Player",80,275,Red,120,100);
		Red=0;

		if (m_SelectY==MM_PROFILES)
			Red=255;
		XEngine->DrawMenuText("Profiles",80,325,Red,120,100);
		Red=0;

		if (m_SelectY==MM_PLAYMOVIE)
			Red=255;
		XEngine->DrawMenuText("Play Movies",80,350,Red,120,100);
		Red=0;
	}
	else if (m_CurrentMenu==2)
	{
		XEngine->DrawMenuText("Profiles",50,100,0,0,255);
		//Y image
		XEngine->DrawMenuText("No Profile",100,400,100,180,0);
		//X image
		XEngine->DrawMenuText("Load Profile",400,400,100,180,0);
		
		Red=0;
		if (m_NumberofPlayers==1)
		{
			m_XposDist=1.0f;
		}
		else if (m_NumberofPlayers==2)
		{
			m_XposDist=.5f;
		}
		else if (m_NumberofPlayers==3)
		{
			m_XposDist=(float(1/3));
		}
		else if (m_NumberofPlayers==4)
		{
			m_XposDist=.25f;
		}
		XEngine->DrawMenuText("New Profile", 400,400,100,180,0);
		//Y image

		for(int m_PlayerNumber=m_NumberofPlayers; m_PlayerNumber>=0; m_PlayerNumber--)
		{
			 
			if (m_DispChars[m_PlayerNumber] || m_ReadyBegin[m_PlayerNumber] || m_DispProf[m_PlayerNumber])
			{
				//draws profiles list
				
				if (m_DispChars[m_PlayerNumber])
				{
					char CharText[2];
					//XEngine->DrawMenuText(Game.m_GameSettings->Profiles[m_PlayerNumber]->ProfileName,(int((m_XposDist) * 120)),150,255,120,100);
		
					CharText[0]=(char)ToLetters();
					CharText[1]=NULL;
					XEngine->DrawMenuText(CharText, (int((m_XposDist) * 120)),250,255,120,100);
					int NumTemp=m_SelectY;
					for (int ii=1; ii<=2; ii++)
					{
						m_SelectY-=ii;
						if (m_SelectY<0)
						{
							m_SelectY=36;
						}

						CharText[0]=(char)ToLetters();
						CharText[1]=NULL;
		
						XEngine->DrawMenuText(CharText, (int((m_XposDist) * 120)),(250-((ii)*25)),0,120,100);

						m_SelectY=NumTemp;
						m_SelectY+=ii;
						if (m_SelectY>36)
						{
							m_SelectY=0;
						}

						CharText[0]=(char)ToLetters();
						CharText[1]=NULL;
		
						XEngine->DrawMenuText(CharText, (int((m_XposDist) * 120)),(250-((ii)*25)),0,120,100);
					}
					m_SelectY=NumTemp;
				}
				
				else if(m_DispProf[m_PlayerNumber])
				{
					int Blue=0;
					char PrintCharacter[256];
					if (m_SelectY==0)
					{
						Blue=255;
					}
					else
					{
						Blue=0;
					}

					if (Game.m_GameSettings->Profiles[m_PlayerNumber]->ProfileName==NULL)
					{
						XEngine->DrawText("NO NAME YET",(int((m_XposDist) * 120)),200,255,120,Blue);
					}
					else
					{
						XEngine->DrawText(Game.m_GameSettings->Profiles[m_PlayerNumber]->ProfileName,(int((m_XposDist) * 120)),200,255,120,Blue);
					}

					XEngine->DrawSprite(m_ProfileColor[Game.m_GameSettings->Profiles[m_PlayerNumber]->ProfileColor],(float((m_XposDist) * 120)),165,64,64,0);
					if (m_SelectY==1)
					{
						//XEngine->DrawSprite(m_ArrowRight,(float((m_XposDist) * 120)+70),165,64,64,0);
						//XEngine->DrawSprite(m_ArrowRight,(float((m_XposDist) * 120)-15),165,64,64,90);
					}

					if (m_SelectY==2)
					{
						Blue=255;
					}
					else
					{
						Blue=0;
					}
					XEngine->DrawText("Sensitivity",(int((m_XposDist) * 120)),225,255,120,Blue);
					XEngine->DrawText("x",(int((m_XposDist) * 120)+110),225,255,120,Blue);
					for (int m_Counter=1; m_Counter<=100; m_Counter++)
					{
						if (Game.m_GameSettings->Profiles[m_PlayerNumber]->JoystickSensitivity.x=float(m_Counter))
						{
							sprintf(PrintCharacter,"%i",m_Counter);
							XEngine->DrawText(PrintCharacter,(int((m_XposDist) * 120)+112),225,255,120,Blue);
						}
					}
					//XEngine->DrawText(PrintCharacter,((int((m_XposDist) * 120))+90),225,255,120,0);

					if (m_SelectY==3)
					{
						Blue=255;
					}
					else
					{
						Blue=0;
					}
					XEngine->DrawText("Invert Y Axis",(int((m_XposDist) * 120)),250,255,120,Blue);
					if(Game.m_GameSettings->Profiles[m_PlayerNumber]->InvertYAxis=false)
						XEngine->DrawText("Yes",(int((m_XposDist) * 120)+150),250,255,120,Blue);
					else
						XEngine->DrawText("No",(int((m_XposDist) * 120)+150),250,255,120,Blue);

					if (m_SelectY==4)
					{
						Blue=255;
					}
					else
					{
						Blue=0;
					}
					XEngine->DrawText("Cursor Follow",(int((m_XposDist) * 120)),275,255,120,Blue);
					if(Game.m_GameSettings->Profiles[m_PlayerNumber]->RelativeAiming=false)
						XEngine->DrawText("Yes",(int((m_XposDist) * 120)+170),275,255,120,Blue);
					else
						XEngine->DrawText("No",(int((m_XposDist) * 120)+170),275,255,120,Blue);
					
				}
			}
			if (m_NumberofPlayers==1)
			{
				m_XposDist=1.0f;
			}
			else if (m_NumberofPlayers==2)
			{
				m_XposDist+=.5f;
			}
			else if (m_NumberofPlayers==3)
			{
				if (m_PlayerNumber==0)
				{
					m_XposDist=1;
				}
				else
				{
					m_XposDist+=(float(1/3));
				}
			}
			else if (m_NumberofPlayers==4)
			{
				m_XposDist+=.25f;
			}
		}
	}
	else if (m_CurrentMenu==8)
	{	
		
		//XCreateSaveGame(

	}
	XEngine->XUpdate();

}
*/