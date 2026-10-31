#include "xengine.h"
#include "myfactory.h"

CXEngine *XEngine=new CXEngine;

void CSound::LoadSound(char* filename)
{	
	XEngine->m_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment, IID_IDirectMusicSegment8,
                                  filename, (VOID**)&m_pSoundSegment);
}

void CSound::Create(char* filename)
{
	LoadSound(filename);
}

void CSound::PlaySound()
{
	if (XEngine->m_SoundsPerFrame > 0)
	{
		XEngine->m_pPerformance->PlaySegmentEx( m_pSoundSegment, NULL, NULL, DMUS_SEGF_SECONDARY,0, NULL, NULL, XEngine->m_MusicAudioPath);
		XEngine->m_SoundsPerFrame--;
	}
}

void CSound::PlaySound(IDirectMusicSegmentState **Channel)
{
	XEngine->m_pPerformance->PlaySegmentEx( m_pSoundSegment, NULL, NULL, DMUS_SEGF_SECONDARY,0, Channel, NULL, XEngine->m_MusicAudioPath);
	//XEngine->m_pPerformance->PlaySegmentEx( m_pSoundSegment, NULL, NULL, DMUS_SEGF_SECONDARY,0, NULL, NULL, XEngine->m_MusicAudioPath);
}

void CSound::SetSoundType(int Type)
{
	switch(Type)
	{
	case ST_NORMAL:

		break;

	case ST_LOOPING:

		m_pSoundSegment->SetRepeats( DMUS_SEG_REPEAT_INFINITE );
		break;
	}
}

void CXEngine::StopChannel(IDirectMusicSegmentState *Channel)
{
	m_pPerformance->StopEx(Channel,0,0);
}

void CSound::Release()
{
	m_pSoundSegment->Release();
	
}

void CXEngine::GraphicsInit()
{
	//d3d init
	m_pD3D = Direct3DCreate8(D3D_SDK_VERSION);

	//display init
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = 640;
	d3dpp.BackBufferHeight = 480;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.EnableAutoDepthStencil=TRUE;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D24S8;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	m_pD3D->CreateDevice(0,D3DDEVTYPE_HAL,NULL,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp,&m_pD3DDevice);

	//default render states
	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	m_pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	m_pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000);

	//m_pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_CIO);

	m_pD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	m_pD3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_CURRENT);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);

	//default texture stage states
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	m_pD3DDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_MIPFILTER,D3DTEXF_POINT);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pD3DDevice->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_MIPFILTER,D3DTEXF_POINT);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pD3DDevice->SetTextureStageState(2,D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_MIPFILTER,D3DTEXF_POINT);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pD3DDevice->SetTextureStageState(3,D3DTSS_COLORARG1, D3DTA_TEXTURE);

	//build sprite vertex buffer
    m_pD3DDevice->CreateVertexBuffer(4*sizeof(SPRITEVERTEX),D3DUSAGE_WRITEONLY,D3DFVF_XYZ|D3DFVF_TEX1,D3DPOOL_DEFAULT,&m_SpriteVertexBuffer);
 
	SPRITEVERTEX* pSpriteVertices;

    m_SpriteVertexBuffer->Lock(0,0,(BYTE **)&pSpriteVertices,0L);

    pSpriteVertices[0].Pos = D3DXVECTOR3( -0.5f, +0.5f, 10.0f);  pSpriteVertices[0].Tex = D3DXVECTOR2( 0.0f, 0.0f );
	pSpriteVertices[1].Pos = D3DXVECTOR3( +0.5f, +0.5f, 10.0f);  pSpriteVertices[2].Tex = D3DXVECTOR2( 0.0f, 1.0f );
    pSpriteVertices[2].Pos = D3DXVECTOR3( -0.5f, -0.5f, 10.0f);  pSpriteVertices[1].Tex = D3DXVECTOR2( 1.0f, 0.0f );
    pSpriteVertices[3].Pos = D3DXVECTOR3( +0.5f, -0.5f, 10.0f);  pSpriteVertices[3].Tex = D3DXVECTOR2( 1.0f, 1.0f );

    m_SpriteVertexBuffer->Unlock();

	//may need to be dynamic
	m_pD3DDevice->SetVertexShader(D3DFVF_XYZ|D3DFVF_TEX1);
    m_pD3DDevice->SetStreamSource(0,m_SpriteVertexBuffer,sizeof(SPRITEVERTEX));

	//and projection matrix, this needs to be corrected
	D3DXMatrixOrthoOffCenterLH(&m_matProj,-20.0f,640.0f+20.0f,-20.0f,480.0f+20.0f,1.0f,1000.0f);
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION,&m_matProj);

	//load some fonts since I can't think of any better place to put this
	//XFONT_OpenBitmapFont(L"D:\\fonts\\dokka24.bmf",16*1024, &m_pDokka24Font);
	XFONT_OpenBitmapFont(L"D:\\fonts\\microfont36.bmf",16*1024, &m_pMicroFont36Font);
	XFONT_OpenBitmapFont(L"D:\\fonts\\microfont24.bmf",16*1024, &m_pMicroFont24Font);
	XFONT_OpenBitmapFont(L"D:\\fonts\\microfont18.bmf",16*1024, &m_pMicroFont18Font);

	//get the backbuffer surface
	m_pD3DDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&m_BackBuffer);

	LPDIRECT3DSURFACE8 pRenderTarget;
	m_pD3DDevice->GetRenderTarget(&pRenderTarget);
	
	D3DSURFACE_DESC descRenderTarget;
    pRenderTarget->GetDesc(&descRenderTarget);
    UINT Width = descRenderTarget.Width;
    UINT Height = descRenderTarget.Height;
	D3DFORMAT Format = descRenderTarget.Format;

	//m_pD3DDevice->CreateTexture(1024,1024,0,0,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&m_MotionBlurTexture);
	//m_pD3DDevice->CreateTexture(1024, 1024, 1,0,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, &m_MotionBlurTexture);

	//EnableMotionBlur();
	//m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	//DisableMotionBlur();

	//initialize the wmv player
//	m_WMVPlayer.Initialize(m_pD3DDevice, m_pDSound);
}

void CXEngine::GraphicsUpdate()
{
	m_pD3DDevice->EndScene();
	m_pD3DDevice->Present(NULL, NULL, NULL, NULL);

	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	m_pD3DDevice->BeginScene();
}

void CXEngine::DownloadEffectsImage(PCHAR pszScratchFile)
{
    HANDLE hFile;
    DWORD dwSize;
    PVOID pBuffer = NULL;
    HRESULT hr = S_OK;
    LPDSEFFECTIMAGEDESC pDesc;
    DSEFFECTIMAGELOC EffectLoc;

    // open scratch image file generated by xps2 tool
    hFile = CreateFile( pszScratchFile,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );

 
        // Determine the size of the scratch image by seeking to
        // the end of the file
        dwSize = SetFilePointer( hFile, 0, NULL, FILE_END );
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    
        // Allocate memory to read the scratch image from disk
        pBuffer = new BYTE[dwSize];

        // Read the image in
        DWORD dwBytesRead;
        BOOL bResult = ReadFile( hFile,
                                 pBuffer,
                                 dwSize,
                                 &dwBytesRead,
                                 0 );
        

 
        // call DSound API to download the image..
        EffectLoc.dwI3DL2ReverbIndex = I3DL2_CHAIN_I3DL2_REVERB;
        EffectLoc.dwCrosstalkIndex = I3DL2_CHAIN_XTALK;

        hr = m_pDSound->DownloadEffectsImage( pBuffer,
                                              dwSize,
                                              &EffectLoc,
                                              &pDesc );

    delete[] pBuffer;

	CloseHandle( hFile );
}

void CXEngine::SoundInit()
{
	//init directsound
	DirectSoundCreate(NULL, &m_pDSound,NULL);

	DirectSoundUseFullHRTF();

	DownloadEffectsImage("d:\\dsstdfx.bin");

	//init directmusic
	IDirectMusicHeap* pNormalHeap;
	DirectMusicCreateDefaultHeap( &pNormalHeap );

	IDirectMusicHeap* pPhysicalHeap;
	DirectMusicCreateDefaultPhysicalHeap( &pPhysicalHeap );

	DirectMusicInitializeEx(pNormalHeap,pPhysicalHeap,MyFactory);

	pNormalHeap->Release();
	pPhysicalHeap->Release();
	
    DirectMusicCreateInstance( CLSID_DirectMusicLoader, NULL,
                               IID_IDirectMusicLoader8, (VOID**)&m_pLoader );

	DirectMusicCreateInstance( CLSID_DirectMusicPerformance, NULL,
                               IID_IDirectMusicPerformance8, (VOID**)&m_pPerformance );

	m_pPerformance->InitAudioX(DMUS_APATH_DYNAMIC_3D, 64, 128, 0 );

	m_MusicThread=NULL;//temp
	m_MusicPlaying=false;

	//m_pPerformance->CreateStandardAudioPath(DMUS_APATH_SHARED_STEREOPLUSREVERB, 128, TRUE, &m_MusicAudioPath);
	//m_MusicAudioPath->Activate(true);

	m_pPerformance->GetDefaultAudioPath(&m_MusicAudioPath);
	m_MusicAudioPath->SetVolume(-1000,0);
}

void CXEngine::SetI3DL2Listener(LPCDSI3DL2LISTENER pds3dl)
{
	m_pDSound->SetI3DL2Listener(pds3dl,DS3D_IMMEDIATE);
}

void CXEngine::SoundUpdate()
{
	m_SoundsPerFrame = 2;

	DirectSoundDoWork();
}

void CXEngine::TimingInit()
{
	m_Tween=1.0f;
	m_Ticks=0.0f;
	m_GameSpeed=1.0f;

	QueryPerformanceCounter(&m_Millisecs);
}

void CXEngine::TimingUpdate()
{
	m_LastMillisecs.QuadPart = m_Millisecs.QuadPart;
	QueryPerformanceCounter(&m_Millisecs);

	m_Tween = (m_Millisecs.QuadPart-m_LastMillisecs.QuadPart)/(733333333.0f/60.0f)*m_GameSpeed;
	m_Ticks += m_Tween;

	m_FPS = ((float)60.0/(float)m_Tween);
}

void CXEngine::InputInit()
{
	XInitDevices(0,0);

    DWORD dwDeviceMask = XGetDevices( XDEVICE_TYPE_GAMEPAD );

    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        ZeroMemory( &InputStates[i], sizeof(XINPUT_STATE) );
        ZeroMemory( &GamePad[i], sizeof(XBGAMEPAD) );
        if( dwDeviceMask & (1<<i) ) 
        {
            GamePad[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                                XDEVICE_NO_SLOT, NULL );

            XInputGetCapabilities(GamePad[i].hDevice, &GamePad[i].caps);
        }
    }
}

void CXEngine::InputUpdate()
{
    DWORD dwInsertions, dwRemovals;
    XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals );

    // Loop through all gamepads
    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        // Handle removed devices.
        GamePad[i].bRemoved = ( dwRemovals & (1<<i) ) ? TRUE : FALSE;
        if( GamePad[i].bRemoved )
        {
            // if the controller was removed after XGetDeviceChanges but before
            // XInputOpen, the device handle will be NULL
            if( GamePad[i].hDevice )
                XInputClose( GamePad[i].hDevice );
            GamePad[i].hDevice = NULL;
            GamePad[i].Feedback.Rumble.wLeftMotorSpeed  = 0;
            GamePad[i].Feedback.Rumble.wRightMotorSpeed = 0;
        }
		
        // Handle inserted devices
        GamePad[i].bInserted = ( dwInsertions & (1<<i) ) ? TRUE : FALSE;
        if( GamePad[i].bInserted ) 
        {
            // TCR 1-14 Device Types
            GamePad[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                               XDEVICE_NO_SLOT, NULL );

            // if the controller is removed after XGetDeviceChanges but before
            // XInputOpen, the device handle will be NULL
            if( GamePad[i].hDevice )
                XInputGetCapabilities( GamePad[i].hDevice, &GamePad[i].caps );
        }

        // If we have a valid device, poll it's state and track button changes
        if( GamePad[i].hDevice )
        {
            // Read the input state
            XInputGetState( GamePad[i].hDevice, &InputStates[i] );
			
            // Copy gamepad to local structure
            memcpy( &GamePad[i], &InputStates[i].Gamepad, sizeof(XINPUT_GAMEPAD) );

            // Put Xbox device input for the gamepad into our custom format
            FLOAT fX1 = (GamePad[i].sThumbLX+0.5f)/32767.5f;
            GamePad[i].fX1 = ( fX1 >= 0.0f ? 1.0f : -1.0f ) *
                               max( 0.0f, (fabsf(fX1)-XE_XBINPUT_DEADZONE)/(1.0f-XE_XBINPUT_DEADZONE) );

            FLOAT fY1 = (GamePad[i].sThumbLY+0.5f)/32767.5f;
            GamePad[i].fY1 = ( fY1 >= 0.0f ? 1.0f : -1.0f ) *
                               max( 0.0f, (fabsf(fY1)-XE_XBINPUT_DEADZONE)/(1.0f-XE_XBINPUT_DEADZONE) );

            FLOAT fX2 = (GamePad[i].sThumbRX+0.5f)/32767.5f;
            GamePad[i].fX2 = ( fX2 >= 0.0f ? 1.0f : -1.0f ) *
                               max( 0.0f, (fabsf(fX2)-XE_XBINPUT_DEADZONE)/(1.0f-XE_XBINPUT_DEADZONE) );

            FLOAT fY2 = (GamePad[i].sThumbRY+0.5f)/32767.5f;
            GamePad[i].fY2 = ( fY2 >= 0.0f ? 1.0f : -1.0f ) *
                               max( 0.0f, (fabsf(fY2)-XE_XBINPUT_DEADZONE)/(1.0f-XE_XBINPUT_DEADZONE) );

            // Get the boolean buttons that have been pressed since the last
            // call. Each button is represented by one bit.
            GamePad[i].wPressedButtons = ( GamePad[i].wLastButtons ^ GamePad[i].wButtons ) & GamePad[i].wButtons;
            GamePad[i].wLastButtons    = GamePad[i].wButtons;

            // Get the analog buttons that have been pressed or released since
            // the last call.
            for( DWORD b=0; b<8; b++ )
            {
                // Turn the 8-bit polled value into a boolean value
                BOOL bPressed = ( GamePad[i].bAnalogButtons[b] > XINPUT_GAMEPAD_MAX_CROSSTALK );

                if( bPressed )
                    GamePad[i].bPressedAnalogButtons[b] = !GamePad[i].bLastAnalogButtons[b];
                else
                    GamePad[i].bPressedAnalogButtons[b] = FALSE;
                
                // Store the current state for the next time
                GamePad[i].bLastAnalogButtons[b] = bPressed;
            }
        }
    }
}

void CXEngine::XUpdate()
{
	GraphicsUpdate();
	TimingUpdate();
	InputUpdate();
	SoundUpdate();
}

void CXEngine::XInit()
{
	SoundInit();
	GraphicsInit();
	TimingInit();
	InputInit();
}

IDirect3DTexture8* CXEngine::LoadTexture(char *pSrcFile)
{
	static IDirect3DTexture8* pTex;
	D3DXCreateTextureFromFileEx(m_pD3DDevice,pSrcFile,0,0,0,0,D3DFMT_UNKNOWN,D3DPOOL_DEFAULT,D3DX_FILTER_BOX,D3DX_FILTER_LINEAR,0xFF000000,NULL,NULL,&pTex);

	//m_pD3DDevice->SetTextureStageState(D3DTEXF_NONE

	return pTex;
}

void CXEngine::SetSpriteMode(int SpriteMode)
{
	switch(SpriteMode)
	{
	case SM_ALPHA_BLEND:

		m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		break;

	case SM_ALPHA_ADD:

		m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		break;
	}
}

void CXEngine::EnableMotionBlur()
{
	LPDIRECT3DSURFACE8 BlurSurface;
	m_MotionBlurTexture->GetSurfaceLevel(0,&BlurSurface);

	m_pD3DDevice->SetRenderTarget(BlurSurface,NULL);
}

void CXEngine::DisableMotionBlur()
{
	m_pD3DDevice->SetRenderTarget(m_BackBuffer,NULL);
}

D3DFORMAT __cdecl MapSwizzledToLinearFormat( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_A1R5G5B5:   return D3DFMT_LIN_A1R5G5B5;
        case D3DFMT_A4R4G4B4:   return D3DFMT_LIN_A4R4G4B4;
        case D3DFMT_A8:         return D3DFMT_LIN_A8;
        case D3DFMT_A8B8G8R8:   return D3DFMT_LIN_A8B8G8R8;
        case D3DFMT_A8R8G8B8:   return D3DFMT_LIN_A8R8G8B8;
        case D3DFMT_B8G8R8A8:   return D3DFMT_LIN_B8G8R8A8;
        case D3DFMT_G8B8:       return D3DFMT_LIN_G8B8;
        case D3DFMT_R4G4B4A4:   return D3DFMT_LIN_R4G4B4A4;
        case D3DFMT_R5G5B5A1:   return D3DFMT_LIN_R5G5B5A1;
        case D3DFMT_R5G6B5:     return D3DFMT_LIN_R5G6B5;
        case D3DFMT_R6G5B5:     return D3DFMT_LIN_R6G5B5;
        case D3DFMT_R8B8:       return D3DFMT_LIN_R8B8;
        case D3DFMT_R8G8B8A8:   return D3DFMT_LIN_R8G8B8A8;
        case D3DFMT_X1R5G5B5:   return D3DFMT_LIN_X1R5G5B5;
        case D3DFMT_X8R8G8B8:   return D3DFMT_LIN_X8R8G8B8;
        case D3DFMT_A8L8:       return D3DFMT_LIN_A8L8;
        case D3DFMT_AL8:        return D3DFMT_LIN_AL8;
        case D3DFMT_L16:        return D3DFMT_LIN_L16;
        case D3DFMT_L8:         return D3DFMT_LIN_L8;
        case D3DFMT_V16U16:     return D3DFMT_LIN_V16U16;
//These constants have same value as other constants above.
//        case D3DFMT_V8U8:       return D3DFMT_LIN_V8U8;
//        case D3DFMT_L6V5U5:     return D3DFMT_LIN_L6V5U5;
//        case D3DFMT_X8L8V8U8:   return D3DFMT_LIN_X8L8V8U8;
//        case D3DFMT_Q8W8V8U8:   return D3DFMT_LIN_Q8W8V8U8;
        case D3DFMT_D24S8:      return D3DFMT_LIN_D24S8;
        case D3DFMT_F24S8:      return D3DFMT_LIN_F24S8;
        case D3DFMT_D16:        return D3DFMT_LIN_D16;
        case D3DFMT_F16:        return D3DFMT_LIN_F16;
        default:
            return fmt;
    }
}

HRESULT CXEngine::CopyRects(D3DSurface *pSurfaceSrc, CONST RECT *pSrcRectsArray, UINT cRects, D3DSurface *pSurfaceDst, CONST POINT *pDstPointsArray)
{
    HRESULT hr;
    D3DSURFACE_DESC descSrc;
    pSurfaceSrc->GetDesc(&descSrc);
    BOOL bSwizzledSrc = XGIsSwizzledFormat(descSrc.Format);
    D3DSURFACE_DESC descDst;
    pSurfaceDst->GetDesc(&descDst);
    BOOL bSwizzledDst = XGIsSwizzledFormat(descDst.Format);
    if (! bSwizzledSrc && !bSwizzledDst ) // if formats are both unswizzled, use regular CopyRects
        return m_pD3DDevice->CopyRects(pSurfaceSrc, pSrcRectsArray, cRects, pSurfaceDst, pDstPointsArray);
    if (MapSwizzledToLinearFormat(descSrc.Format) != MapSwizzledToLinearFormat(descDst.Format))
        return D3DERR_WRONGTEXTUREFORMAT;

    // Otherwise, copy pixel by pixel
    DWORD cbPixel = XGBytesPerPixelFromFormat(descSrc.Format);
    D3DLOCKED_RECT lockedRectSrc;
    hr = pSurfaceSrc->LockRect(&lockedRectSrc, NULL, 0);
    if (FAILED(hr))
        return hr;
    D3DLOCKED_RECT lockedRectDst;
    hr = pSurfaceDst->LockRect(&lockedRectDst, NULL, 0);
    if (FAILED(hr))
    {
        pSurfaceSrc->UnlockRect();
        return hr;
    }
    for (UINT iRect = 0; iRect < cRects; iRect++)
    {
        RECT rectSrc;
        if (pSrcRectsArray)
            rectSrc = pSrcRectsArray[iRect];
        else
        {
            rectSrc.left = 0;
            rectSrc.right = descSrc.Width;
            rectSrc.top = 0;
            rectSrc.bottom = descSrc.Height;
        }
        POINT pointDst;
        if (pDstPointsArray)
            pointDst = pDstPointsArray[iRect];
        else
            pointDst.x = pointDst.y = 0;
        if (bSwizzledSrc && bSwizzledDst)
        {
            Swizzler swizSrc(descSrc.Width, descSrc.Height, 0);
            Swizzler swizDst(descDst.Width, descDst.Height, 0);
            swizSrc.SetV(swizSrc.SwizzleV(rectSrc.top));
            swizDst.SetV(swizDst.SwizzleV(pointDst.y));
            for (long v = rectSrc.top; v < rectSrc.bottom; v++)
            {
                swizSrc.SetU(swizSrc.SwizzleU(rectSrc.left));
                swizDst.SetU(swizDst.SwizzleU(pointDst.x));
                for (long u = rectSrc.left; u < rectSrc.right; u++)
                {
                    BYTE *pSrc = (BYTE *)lockedRectSrc.pBits + cbPixel * swizSrc.Get2D();
                    BYTE *pDst = (BYTE *)lockedRectDst.pBits + cbPixel * swizDst.Get2D();
                    memcpy(pDst, pSrc, cbPixel);
                    swizSrc.IncU();
                    swizDst.IncU();
                }
                swizSrc.IncV();
                swizDst.IncV();
            }
        }
        else if (bSwizzledSrc) // && !bSwizzledDst
        {
            Swizzler swizSrc(descSrc.Width, descSrc.Height, 0);
            swizSrc.SetV(swizSrc.SwizzleV(rectSrc.top));
            for (long v = rectSrc.top; v < rectSrc.bottom; v++)
            {
                long vDst = pointDst.y + v - rectSrc.top;
                swizSrc.SetU(swizSrc.SwizzleU(rectSrc.left));
                for (long u = rectSrc.left; u < rectSrc.right; u++)
                {
                    long uDst = pointDst.x + u - rectSrc.left;
                    BYTE *pSrc = (BYTE *)lockedRectSrc.pBits + cbPixel * swizSrc.Get2D();
                    BYTE *pDst = (BYTE *)lockedRectDst.pBits + vDst * lockedRectDst.Pitch + uDst * cbPixel;
                    memcpy(pDst, pSrc, cbPixel);
                    swizSrc.IncU();
                }
                swizSrc.IncV();
            }
        }
        else // !bSwizzledSrc && bSwizzledDst
        {
            Swizzler swizDst(descDst.Width, descDst.Height, 0);
            swizDst.SetV(swizDst.SwizzleV(pointDst.y));
            for (long v = rectSrc.top; v < rectSrc.bottom; v++)
            {
                swizDst.SetU(swizDst.SwizzleU(pointDst.x));
                for (long u = rectSrc.left; u < rectSrc.right; u++)
                {
                    BYTE *pSrc = (BYTE *)lockedRectSrc.pBits + v * lockedRectSrc.Pitch + u * cbPixel;
                    BYTE *pDst = (BYTE *)lockedRectDst.pBits + cbPixel * swizDst.Get2D();
                    memcpy(pDst, pSrc, cbPixel);
                    swizDst.IncU();
                }
                swizDst.IncV();
            }
        }
    }
    pSurfaceSrc->UnlockRect();
    pSurfaceDst->UnlockRect();
    return S_OK;
}

void CXEngine::RenderMotionBlur(float Alpha)
{
	POINT DestPoint;

	DestPoint.x=0;
	DestPoint.y=0;

	RECT SourceRect;

	SourceRect.top=0;
	SourceRect.bottom=480;
	SourceRect.left=0;
	SourceRect.right=640;
	
	LPDIRECT3DSURFACE8 m_BlurSurface;
	m_MotionBlurTexture->GetSurfaceLevel(0,&m_BlurSurface);
	
	D3DLOCKED_RECT LockedRectSrc,LockedRectDst;
	m_BackBuffer->LockRect(&LockedRectSrc,NULL,0);
	m_BlurSurface->LockRect(&LockedRectDst,NULL,0);

	//char TempBuffer[640*480*4];
	char *TempBuffer=new char[640*480*4];
	
	XGUnswizzleRect(LockedRectSrc.pBits,640,480,&SourceRect,TempBuffer,640/4,&DestPoint,4);
	XGSwizzleRect(TempBuffer,640/4,&SourceRect,LockedRectDst.pBits,1024,1024,&DestPoint,4);
	//XGSwizzleRect(LockedRectSrc.pBits,LockedRectSrc.Pitch,&SourceRect,LockedRectDst.pBits,1024,1024,&DestPoint,4);

	m_BackBuffer->UnlockRect();
	m_BlurSurface->UnlockRect();	

	delete TempBuffer;

	SetSpriteMode(SM_ALPHA_ADD);
	SetMaterial(1.0f,1.0f,1.0f,Alpha);

	DrawSprite(m_MotionBlurTexture,320.0f,240.0f,693.0f,520.0f,0.0f);
}

void CXEngine::SetMaterial(float Red,float Green,float Blue,float Alpha)
{
	m_pD3DDevice->SetVertexData4f(D3DVSDE_DIFFUSE,Red,Green,Blue,Alpha);
}

void CXEngine::DrawSprite(IDirect3DTexture8* Texture,float PosX,float PosY,float SizeX,float SizeY,float Rot)
{
	D3DXMATRIX PosMatrix,ScaleMatrix,RotMatrix,FinalMatrix;

	D3DXMatrixTranslation(&PosMatrix,PosX,480.0f-PosY,0.0f);
	D3DXMatrixScaling(&ScaleMatrix,SizeX,SizeY,1.0f);
	D3DXMatrixRotationZ(&RotMatrix,D3DXToRadian(Rot));

	D3DXMatrixMultiply(&FinalMatrix,&ScaleMatrix,&RotMatrix);
	D3DXMatrixMultiply(&FinalMatrix,&FinalMatrix,&PosMatrix);

	//D3DXMatrixMultiply(&FinalMatrix,&ScaleMatrix,&PosMatrix);
	//D3DXMatrixMultiply(&FinalMatrix,&RotMatrix,&FinalMatrix);

	m_pD3DDevice->SetTransform(D3DTS_WORLD,&FinalMatrix);

    m_pD3DDevice->SetTexture(0,Texture);
    m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
}

void CXEngine::DrawSpriteOC(IDirect3DTexture8* Texture,float PosX,float PosY,float SizeX,float SizeY,float Rot)
{
	D3DXMATRIX PosMatrix,ScaleMatrix,RotMatrix,FinalMatrix;

	D3DXMatrixTranslation(&PosMatrix,PosX,480.0f-PosY,0.0f);
	D3DXMatrixScaling(&ScaleMatrix,SizeX,SizeY,1.0f);
	D3DXMatrixRotationZ(&RotMatrix,D3DXToRadian(Rot));

	D3DXMatrixMultiply(&FinalMatrix,&ScaleMatrix,&PosMatrix);
	D3DXMatrixMultiply(&FinalMatrix,&RotMatrix,&FinalMatrix);

	D3DXMatrixTranslation(&PosMatrix,-0.35f,-0.35f,0.0f);
	D3DXMatrixMultiply(&FinalMatrix,&PosMatrix,&FinalMatrix);

	m_pD3DDevice->SetTransform(D3DTS_WORLD,&FinalMatrix);

    m_pD3DDevice->SetTexture(0,Texture);
    m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
}

void CXEngine::DrawTracer(IDirect3DTexture8* Texture,float X1,float Y1,float X2,float Y2,float Width)
{
	D3DXMATRIX WorldMatrix;
	D3DXMatrixIdentity(&WorldMatrix);
	m_pD3DDevice->SetTransform(D3DTS_WORLD,&WorldMatrix);

	m_pD3DDevice->SetTexture(0,Texture);

	SPRITEVERTEX TracerVerts[4];

	float Theta=ATan2(X2-X1,Y2-Y1);
	float DeltaX=Sin(Theta-90.0f),DeltaY=Cos(Theta-90.0f);

	TracerVerts[0].Pos.x=X1-DeltaX*Width;
	TracerVerts[0].Pos.y=480.0f-(Y1-DeltaY*Width);
	TracerVerts[0].Pos.z=10.0f;
	TracerVerts[0].Tex.x=0.0f;
	TracerVerts[0].Tex.y=1.0f;

	TracerVerts[1].Pos.x=X1+DeltaX*Width;
	TracerVerts[1].Pos.y=480.0f-(Y1+DeltaY*Width);
	TracerVerts[1].Pos.z=10.0f;
	TracerVerts[1].Tex.x=1.0f;
	TracerVerts[1].Tex.y=1.0f;

	TracerVerts[2].Pos.x=X2-DeltaX*Width;
	TracerVerts[2].Pos.y=480.0f-(Y2-DeltaY*Width);
	TracerVerts[2].Pos.z=10.0f;
	TracerVerts[2].Tex.x=0.0f;
	TracerVerts[2].Tex.y=0.0f;

	TracerVerts[3].Pos.x=X2+DeltaX*Width;
	TracerVerts[3].Pos.y=480.0f-(Y2+DeltaY*Width);
	TracerVerts[3].Pos.z=10.0f;
	TracerVerts[3].Tex.x=1.0f;
	TracerVerts[3].Tex.y=0.0f;
	
	m_pD3DDevice->DrawVerticesUP(D3DPT_TRIANGLESTRIP,4,TracerVerts,sizeof(SPRITEVERTEX));
}

void CXEngine::DrawText(char *strText,int PosX,int PosY,int Red,int Green,int Blue)
{
	WCHAR szbuff[256] = {0};
	swprintf(szbuff,L"%hs",strText);

	m_pMicroFont24Font->SetTextColor(D3DCOLOR_XRGB(Red,Green,Blue));

	m_pMicroFont24Font->TextOut(m_BackBuffer,szbuff,-1,PosX-1,PosY);
	m_pMicroFont24Font->TextOut(m_BackBuffer,szbuff,-1,PosX+1,PosY);
	m_pMicroFont24Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY-1);
	m_pMicroFont24Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY+1);

	
	m_pMicroFont24Font->SetTextColor(D3DCOLOR_XRGB(Red/8,Green/8,Blue/8));
	m_pMicroFont24Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY);
}

void CXEngine::DrawMenuText(char *strText,int PosX,int PosY,int Red,int Green,int Blue)
{
	WCHAR szbuff[256] = {0};
	swprintf(szbuff,L"%hs",strText);

	m_pMicroFont36Font->SetTextColor(D3DCOLOR_XRGB(Red,Green,Blue));

	m_pMicroFont36Font->TextOut(m_BackBuffer,szbuff,-1,PosX-1,PosY);
	m_pMicroFont36Font->TextOut(m_BackBuffer,szbuff,-1,PosX+1,PosY);
	m_pMicroFont36Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY-1);
	m_pMicroFont36Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY+1);

	m_pMicroFont36Font->SetTextColor(D3DCOLOR_XRGB(Red/8,Green/8,Blue/8));
	m_pMicroFont36Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY);
}

void CXEngine::DrawScoreText(char *strText,int PosX,int PosY,int Red,int Green,int Blue)
{
	WCHAR szbuff[256] = {0};
	swprintf(szbuff,L"%hs",strText);

	m_pMicroFont18Font->SetTextColor(D3DCOLOR_XRGB(Red,Green,Blue));
	m_pMicroFont18Font->TextOut(m_BackBuffer,szbuff,-1,PosX,PosY);
}

float Fag = 0.0f;

void CXEngine::SetCameraPos(float PosX,float PosY)
{
	D3DXMATRIX WorldMatrix;
	D3DXMatrixTranslation(&WorldMatrix,PosX,PosY,0.0f);
	//D3DXMatrixTranslation(&WorldMatrix,PosX + 320.0f,PosY + 240.0f,0.0f);

	/*
	D3DXMATRIX RotMatrix;
	D3DXMatrixRotationZ(&RotMatrix, Sin(Fag) * 0.25f);
	D3DXMatrixMultiply(&WorldMatrix,&RotMatrix,&WorldMatrix);
	D3DXMatrixTranslation(&RotMatrix,-320.0f,-240.0f,0.0f);
	D3DXMatrixMultiply(&WorldMatrix,&RotMatrix,&WorldMatrix);

	Fag += m_Tween * 0.5;
*/
	m_pD3DDevice->SetTransform(D3DTS_VIEW,&WorldMatrix);
}

COLLISIONINFO CXEngine::Collide(float X1,float Y1,float R1,float DX,float DY,float X2,float Y2,float R2)
{
	COLLISIONINFO MyCollisionInfo;

	D3DXVECTOR2 D1;

	D1.x=DX;
	D1.y=DY;

	float D1Length=Sqrt(D1.x*D1.x+D1.y*D1.y);

	D1.x/=D1Length;
	D1.y/=D1Length;

	D3DXVECTOR2 D2;

	D2.x=X2-X1;
	D2.y=Y2-Y1;

	float D2Length=Sqrt(D2.x*D2.x+D2.y*D2.y);

	D2.x/=D2Length;
	D2.y/=D2Length;

	float A=(D1.x*D2.x+D1.y*D2.y)*D2Length;
	float O=Sqrt(D2Length*D2Length-A*A);

	float DestX=X1+DX,DestY=Y1+DY;
	float LowX,HighX,LowY,HighY;

	if (DestX<X1)
	{
		LowX=DestX;
		HighX=X1;
	}
	else
	{
		LowX=X1;
		HighX=DestX;
	}

	if (DestY<Y1)
	{
		LowY=DestY;
		HighY=Y1;
	}
	else
	{
		LowY=Y1;
		HighY=DestY;
	}

	LowX=LowX-R1-R2;
	HighX=HighX+R1+R2;
	LowY=LowY-R1-R2;
	HighY=HighY+R1+R2;

	if (O<R1+R2 && X2>LowX && X2<HighX && Y2>LowY && Y2<HighY)
	{
		MyCollisionInfo.Collided=true;

		MyCollisionInfo.Pos.x=D1.x*(A-((R1+R2)-O));//WRONG
		MyCollisionInfo.Pos.y=D1.y*(A-((R1+R2)-O));//WRONG
	}
	else
	{
		MyCollisionInfo.Collided=false;

		MyCollisionInfo.Pos.x=D1.x*D1Length;
		MyCollisionInfo.Pos.y=D1.y*D1Length;
	}

	return MyCollisionInfo;
}


DWORD WINAPI MusicProc( LPVOID lpParameter )
{
    // To process approximately once per frame, we can sleep
    // for 1000 ms / 60 fps between calls to Process.
    //
    // Alternately, the minimum time between processing is
    // determined by how much data we're sending to the stream
    // at once:
    // 2048 * 16 samples per packet * 16 packets = 524288 samples
    // 524288 samples / 44100 samples per second = 11.8 seconds
    //
    // DWORD dwQuantum = 11000;
 
    DWORD dwQuantum = XE_MUSIC_QUANTUM,completed;

    for( ; ; )
    {
        XEngine->m_MusicStream.Process(&completed);
        Sleep( dwQuantum );
    }
}

void CXEngine::PlayMusic(const CHAR* strFileName)
{
	if (m_MusicThread)//temp
	{
		SuspendThread(m_MusicThread);
	}

	m_MusicStream.Initialize(strFileName);
	m_MusicThread = CreateThread( NULL, 0, MusicProc, &m_MusicStream, 0, NULL);

	m_MusicPlaying=true;
}

void CXEngine::PauseMusic()
{
	m_MusicPlaying=!m_MusicPlaying;

	if (m_MusicPlaying)
	{
		ResumeThread(m_MusicThread);
	}
	else
	{
		SuspendThread(m_MusicThread);
	}
}

void CXEngine::SetMusicVolume(int Volume)
{
}

void CXEngine::SetGameSpeed(float Speed)
{
	if (m_MusicPlaying)
	{
		m_MusicStream.m_pRenderFilter->SetPitch(int(log(Speed)*2000.0) - 500);
	}

	m_MusicAudioPath->SetPitch(int(log(Speed)*1000.0) - 200,0);

	m_GameSpeed=Speed;
}

IDirectSoundBuffer8* CXEngine::LoadSound(char *pSrcFile)
{
	static IDirectSoundBuffer8* SoundBuffer;

//	m_pDSound->CreateSoundBuffer(blah,SoundBuffer,NULL);
	
	return SoundBuffer;
}

float Sin(float Angle)
{
	return (float)sin(D3DXToRadian(Angle));
}

float Cos(float Angle)
{
	return (float)cos(D3DXToRadian(Angle));
}

float ATan2(float DeltaX,float DeltaY)
{
	return D3DXToDegree((float)atan2(DeltaX,DeltaY));
}

float fAbs(float Num)
{
	return (float)fabs(Num);
}

float Rand()
{
	return (float)rand()/32767.0f;
}

float Pow(float Num,float Degree)
{
	return (float)pow(Num,Degree);
}

float Sqrt(float Num)
{
	return (float)sqrt(Num);
}

HRESULT CXEngine::PlayVideo(char* strFilename)
{
    // Open the video file to be played
//    if( FAILED( m_WMVPlayer.OpenFile( strFilename ) ) )
//        return 0;

	/*
    // Get information about the video format
    WMVVIDEOINFO videoInfo;
    m_WMVPlayer.GetVideoInfo( &videoInfo );

    // Create two textures based off the video format.  We need to
    // use two textures because the overlay surface reads directly
    // from the texture.  Updating a texture while it's being used
    // by the overlay will cause tearing
    for( int i = 0; i < 2; i++ )
    {
        m_pD3DDevice->CreateTexture( videoInfo.dwWidth,
                                     videoInfo.dwHeight,
                                     1,
                                     0,
                                     D3DFMT_YUY2,
                                     0,
                                     &m_pOverlay[i] );
        m_pOverlay[i]->GetSurfaceLevel( 0, &m_pSurface[i] );
    }
    m_dwCurrent = 0;

    // Set up source and destination rectangles
    // TODO: Adjust for different aspect ratios?
    RECT SrcRect;
    RECT DestRect;

    SrcRect.left    = 0;
    SrcRect.top     = 0;
    SrcRect.right   = videoInfo.dwWidth;
    SrcRect.bottom  = videoInfo.dwHeight;

    DestRect.left   = 0;
    DestRect.top    = 0;
    DestRect.right  = 640;
    DestRect.bottom = 480;

    // Timing statistics
    DWORD dwMaxCycles = 0;
    DWORD dwFrame = 0;
    DWORD dwMaxFrame = 0xFFFFFFFF;

    // Enable the Direct3D overlay surface
    m_pD3DDevice->EnableOverlay( TRUE );

    // Playback video.  This is done in its own loop to avoid wasting time
    // waiting for buffer swaps, etc, when we don't need to.

	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	m_pD3DDevice->Present( NULL, NULL, NULL, NULL );

    for( ; ; )
    {
        // If we're ready for the next frame...
        if( m_WMVPlayer.IsReady() )
        {
            dwFrame++;

            // Get the next frame into our texture
            m_WMVPlayer.GetTexture( m_pOverlay[m_dwCurrent] );

            // Update the overlay surface to use this texture
            m_pD3DDevice->UpdateOverlay( m_pSurface[m_dwCurrent], &SrcRect, &DestRect, FALSE, 0 );

            // Decode the next frame
            if( S_FALSE == m_WMVPlayer.DecodeNext() )
            {
                break;
            }

            // We'll use the other texture next time
            m_dwCurrent ^= 1;
        }
        
		InputUpdate();

		bool SkipMovie=false;

		for (int i=0;i<4;i++)
		{
			if(GamePad[i].wPressedButtons)
			{
				SkipMovie=true;
			}

			for (int b=0;b<8;b++)
			{
				if (GamePad[i].bPressedAnalogButtons[b])
				{
					SkipMovie=true;
				}
			}
		}

		if (SkipMovie)
		{
			break;
		} 
    }

    // We're done!

    // When disabling the overlay, you should wait until just after a 
    // vertical blank to avoid any flickering
    m_pD3DDevice->BlockUntilVerticalBlank();
    m_pD3DDevice->EnableOverlay( FALSE );

    for( int i = 0; i < 2; i++ )
    {
        m_pSurface[i]->Release();
        m_pOverlay[i]->Release();
    }
    m_WMVPlayer.CloseFile();

	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	m_pD3DDevice->Present( NULL, NULL, NULL, NULL );

	*/
    return S_OK;
}
