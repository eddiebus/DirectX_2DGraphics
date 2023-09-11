#pragma once
#pragma comment(lib,"windowscodecs.lib")

#include <wincodec.h>
#include <MyDirectX.h>
#include <vector>
#include <string>


namespace EDX
{
	/// <summary>
	/// Used for decoding images to certain pixel formats
	/// </summary>
	struct ImageDecoder
	{
		ImageDecoder()
		{
			CoInitializeEx(nullptr,COINIT_MULTITHREADED);
			HRESULT HR = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&Factory)
			);

			if (HR != S_OK)
			{
				MessageBox(NULL, L"WIC Factory (Image Decoder) failed to initialize.",L"Error",NULL);
			}

		}
		CComPtr<IWICImagingFactory> Factory;
	}ImageDecoder;

	struct Pixel
	{
		Pixel() {}
		Pixel(float rr, float gg, float bb, float aa)
		{
			r = rr;
			g = gg;
			b = gg; 
			a = aa;
		}
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	struct Image
	{
		Image(int Width, int Height); //A blank Image
		Image(LPCVOID Data, size_t DataSize) //An Image with pre-set data
		{
			if (!ImageDecoder.Factory) { return; } 
			CComPtr<IWICBitmapDecoder> ImageDecode;
			HRESULT HR;
			CComPtr<IWICStream> Stream;
			ImageDecoder.Factory->CreateStream(&Stream);
			Stream->InitializeFromMemory(
				(BYTE*)Data,
				DataSize
			);

			ImageDecoder.Factory->CreateDecoderFromStream(
				Stream, NULL,
				WICDecodeMetadataCacheOnDemand,
				&ImageDecode
			);

			ImageDecode->GetFrameCount(&FrameCount);
			for (int i = 0; i < FrameCount;i++)
			{
				Frame.push_back(nullptr);
			}
			
			for (int i = 0; i < Frame.size(); i++)
			{
				CComPtr<IWICBitmapFrameDecode> InputFrame;
				ImageDecode->GetFrame(i, &InputFrame);
				//Convert it for Texture use
				WICConvertBitmapSource(GUID_WICPixelFormat128bppRGBAFloat, InputFrame, (IWICBitmapSource**) &Frame.at(i));
			}

			Frame.at(0)->GetSize(&Width, &Height);
			if (Width > MaxLength || Height > MaxLength) //Image too big for D3D11 . Shrink it!
			{
				for (int i = 0; i < Frame.size(); i++)
				{
					CComPtr<IWICBitmapScaler> ImageScaler;
					EDX::ImageDecoder.Factory->CreateBitmapScaler(&ImageScaler);
					ImageScaler->Initialize(Frame.at(i), MaxLength, MaxLength, WICBitmapInterpolationModeNearestNeighbor);
					ImageScaler.Release();
				}
				Width = MaxLength;
				Height = MaxLength;
			}
		}

		EDX::Pixel GetPixel(UINT FrameIndex, UINT x, UINT y)
		{
			EDX::Pixel OutputPixel;
			UINT MaxColourValue = (UINT)pow(2, 32);
			
			BYTE* PixelBuffer = new BYTE[16]; //128 bits per pixel;
			ZeroMemory(PixelBuffer, sizeof(BYTE) * 16);
			UINT Channel[4] = { 0,0,0,0 };
			WICRect SelectRect;
			SelectRect.X = x;
			SelectRect.Y = y;
			SelectRect.Width = 1;
			SelectRect.Height = 1;

			if (Frame.size() > 0) //Check if we have images to work with
			{
				if (FrameIndex >= Frame.size()) { FrameIndex = (Frame.size() - 1); }
				UINT Stride = 16 * Width;
				Frame.at(FrameIndex)->CopyPixels(&SelectRect, Stride, sizeof(BYTE) * 16, PixelBuffer);
				UINT ChannelIndexSlice = 4; //2 Bytes per pixel channel
				for (int i = 0; i < 4; i++)
				{
					int Index = ChannelIndexSlice * i;
					float test = 0;
					memcpy(&test, &PixelBuffer[Index], sizeof(BYTE) * ChannelIndexSlice);
					std::wstring debugS = L"";
					debugS += std::to_wstring(test) + L"|" + std::to_wstring(MaxColourValue) + L"\n";
					OutputDebugStringW(debugS.c_str());
				}
			}
			delete[] PixelBuffer; 

			return OutputPixel;
		}
		UINT GetFrames()
		{
			return FrameCount;
		}
		int GetWidth()
		{
			return Width;
		}
		int GetHeight()
		{
			return Height;
		}
		std::vector<CComPtr<IWICBitmapFrameDecode>> Frame;
	private:
		UINT Width;
		UINT Height;
		UINT FrameCount;
		int MaxLength = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	};

	class Texture2DRenderTarget : public RenderTargetBase
	{
	public:
		Texture2DRenderTarget() {}
		Texture2DRenderTarget(ID3D11Texture2D* InputTexture)
		{
			EDX::DXDevice.CreateRenderTargetView(InputTexture, &RTView);
		}
	private:
		CComPtr<ID3D11RenderTargetView> RTView;
	};

	class Texture2D : public RenderTargetBase
	{
	public:
		Texture2D() {}
		Texture2D(EDX::Image& Image, UINT FrameIndex = 0) 
		{
			UINT FrameCount = Image.GetFrames();
			if (FrameIndex >= FrameCount)
			{
				FrameIndex = FrameCount - 1;
			}
			
			UINT PixelCount = Image.GetWidth() * Image.GetHeight();
			UINT BytesPerPixel = 16;
			UINT Stride = BytesPerPixel * Image.GetWidth();
			BYTE* PixelArray = new BYTE[PixelCount *16];
			Image.Frame.at(FrameIndex)->CopyPixels(NULL, Stride, sizeof(BYTE) * (PixelCount * 16), PixelArray);
			EDX::DXDevice.CreateTexture2D(PixelArray, Image.GetWidth(), Image.GetHeight(), &TextureMain);
			EDX::DXDevice.CreateShaderResourceView(TextureMain, &ShaderResource);
			EDX::DXDevice.CreateRenderTargetView(TextureMain, &RenderTargetView);
			delete[] PixelArray;
		}
		Texture2D(int Width, int Height,DXGI_FORMAT ImageFormat = DXGI_FORMAT_R32G32B32A32_FLOAT, UINT BytesPerPixel = 16)
		{
			EDX::DXDevice.CreateTexture2D(nullptr, Width, Height, &TextureMain,ImageFormat,BytesPerPixel);
			EDX::DXDevice.CreateShaderResourceView(TextureMain, &ShaderResource);
			EDX::DXDevice.CreateRenderTargetView(TextureMain, &RenderTargetView);
		}
		Texture2D(BYTE* ImageDatap,UINT ImageWidth,UINT ImageHeight,DXGI_FORMAT ImageFormat,UINT BytesPerPixel)
		{
			EDX::DXDevice.CreateTexture2D(ImageDatap, ImageWidth, ImageHeight, &TextureMain, ImageFormat,BytesPerPixel);
			EDX::DXDevice.CreateShaderResourceView(TextureMain, &ShaderResource);
			EDX::DXDevice.CreateRenderTargetView(TextureMain, &RenderTargetView);
		}
		void Clear(DirectX::XMFLOAT4 Colour)
		{
			EDX::DXDevice.ClearRenderTargetView(Colour,RenderTargetView);
		}
		void SetToPixelShader(UINT ResourceSlot = 0)
		{
			EDX:DXDevice.SetPixelShaderResource(ShaderResource,ResourceSlot);
		}
		void SetAsRenderTarget()
		{
			D3D11_VIEWPORT VP = { 0 };
			VP.TopLeftX = 0;
			VP.TopLeftY = 0;
			VP.MinDepth = 0;
			VP.MaxDepth = 100;
			VP.Width  = GetWidth();
			VP.Height = GetHeight();

			EDX::DXDevice.SetRenderTargetView(RenderTargetView);
			EDX::DXDevice.SetRenderTargetViewPort(VP);

			EDX::Camera2D Camera;
			Camera.ProjectionMatrix.WidthHeight = DirectX::XMFLOAT2(GetWidth(), GetHeight());
		}
		UINT GetWidth() { return Width; }
		UINT GetHeight() { return Height; }

		operator bool()
		{
			if (TextureMain){return true;}
			return false;
		}
		
	private:
		CComPtr<ID3D11Texture2D> TextureMain;
		CComPtr<ID3D11RenderTargetView> RenderTargetView;
		CComPtr<ID3D11ShaderResourceView> ShaderResource = nullptr;
		UINT Width;
		UINT Height;
	};

}