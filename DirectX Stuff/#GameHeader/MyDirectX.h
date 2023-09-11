//My Library for Direct2D graphics
#pragma once
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"DXGI.lib")

#ifndef UNICODE
#define UNICODE
#endif


//Header for DirectX 
#include <dxgi1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <atlbase.h>

namespace EDX
{
	/// <summary>
	/// Empty Matrix
	/// </summary>
	struct Matrix
	{
		/// <summary>
		/// > Returns the matrix <para/>
		/// > Makes Transposes it to be used in hlsl 
		/// </summary>
		/// <returns></returns>
		virtual DirectX::XMMATRIX ReturnMatrix() = 0;
	};
	/// <summary>
	/// Used in Vertex Constant <para/>
	/// Trasnlate into space in the scenes's world
	/// </summary>
	struct WorldMatrix
	{
		DirectX::XMFLOAT3 Rotate	= DirectX::XMFLOAT3(0,0,0);
		DirectX::XMFLOAT3 Scale		= DirectX::XMFLOAT3(1, 1, 1);
		DirectX::XMFLOAT3 Translate = DirectX::XMFLOAT3(0, 0, 0);;

		DirectX::XMMATRIX ReturnMatrix()
		{
			DirectX::XMMATRIX output = DirectX::XMMatrixIdentity();
			//Set the three matrices
			DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(
				Scale.x, Scale.y, Scale.z);
			DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(
				Translate.x, Translate.y, Translate.z);
			DirectX::XMMATRIX RotateMatrix = DirectX::XMMatrixRotationRollPitchYaw(
				Rotate.x, Rotate.y, Rotate.z);

			//combine (multiply the 3 matrices)
			output = DirectX::XMMatrixMultiply(ScaleMatrix, RotateMatrix);
			output = DirectX::XMMatrixMultiply(output, TranslateMatrix);
			output = DirectX::XMMatrixTranspose(output);
			return output;
		}
	};
	/// <summary>
	/// Used in vertex constant. <para/>
	/// Translate from world position -> view position
	/// </summary>
	struct ViewMatrix : public Matrix
	{
		DirectX::XMFLOAT3 Rotate = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 Translate = DirectX::XMFLOAT3(0,0,0);

		DirectX::XMMATRIX ReturnMatrix()
		{
			DirectX::XMMATRIX output = DirectX::XMMatrixIdentity();

			DirectX::XMMATRIX RotateMatrix = DirectX::XMMatrixRotationRollPitchYaw(
				Rotate.x, Rotate.y, -Rotate.z);

			DirectX::XMMATRIX TranslateMatrix = DirectX::XMMatrixTranslation(
				Translate.x, Translate.y, Translate.z);

			output = DirectX::XMMatrixMultiply(TranslateMatrix,RotateMatrix);
			return DirectX::XMMatrixTranspose(output);
		}
	};
	/// <summary>
	/// > Translates from View -> Projection (Screen Coordinates)
	/// </summary>
	struct Projection2DMatrix : public Matrix
	{
		DirectX::XMFLOAT2 WidthHeight;
		DirectX::XMFLOAT2 DepthMinMax;

		DirectX::XMMATRIX ReturnMatrix()
		{
			DirectX::XMMATRIX output;
			output = DirectX::XMMatrixOrthographicLH(WidthHeight.x, WidthHeight.y, DepthMinMax.x, DepthMinMax.y);
			output = DirectX::XMMatrixTranspose(output);
			return output;
		}
	};

	/// <summary>
	/// > Information in a vertex used in shaders
	/// > Contains position, colour and texture coords
	/// </summary>
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 colour;
		DirectX::XMFLOAT2 TexCoord;
	};


	// Vertext Constant
	struct VertexConstant
	{
		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixIdentity();
	};
	/// <summary>
	/// Pixel Constants:<para/>
	/// > Texture Coordinates <para/>
	/// > Alpha Masking <para/>
	/// > Colour Balance <para/>
	/// </summary>
	struct PixelConstant
	{
		DirectX::XMFLOAT2 Texturing = DirectX::XMFLOAT2(0,0);
		DirectX::XMFLOAT2 AlphaMasking  = DirectX::XMFLOAT2(0, 0);
		DirectX::XMFLOAT4 ColourBalance = DirectX::XMFLOAT4(1, 1, 1, 1);
	};
}

namespace EDX
{
	// GPU Represented Device
	struct DirectXDevice
	{
		DirectXDevice(bool Debug = false)
		{
			const D3D_FEATURE_LEVEL featureLevels[] = { //SUpported Versions
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
			};

			if (CreateDXGIFactory2(
				0, __uuidof(IDXGIFactory2), //Unique GUID for object
				(void**)&DXGIFactory) != S_OK)
			{
				MessageBox(NULL, L"DXGI Factory Failed", NULL, NULL);
			}

			UINT CreateFlag;
			if (Debug) { CreateFlag = D3D11_CREATE_DEVICE_DEBUG; }
			else { CreateFlag = NULL; }


			if (D3D11CreateDevice(
				NULL, //DXGI Adapter
				D3D_DRIVER_TYPE_HARDWARE, //Driver Type
				NULL,
				CreateFlag, //Layer Flags
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&MainDevice,
				NULL,
				&DeviceContext) != S_OK
				)
			{
				MessageBox(NULL, L"Direct 3D Device Failed : Check GPU D3D Version Support", NULL, NULL);
			}
			else
				{
					D3D11_RASTERIZER_DESC rDesc;
					CComPtr<ID3D11RasterizerState> rState;
					rDesc.FillMode = D3D11_FILL_SOLID;
					rDesc.CullMode = D3D11_CULL_BACK;
					rDesc.FrontCounterClockwise = FALSE;
					rDesc.DepthBias = 0;
					rDesc.SlopeScaledDepthBias = 0.0f;
					rDesc.DepthBiasClamp = 0.0f;
					rDesc.DepthClipEnable = TRUE;
					rDesc.ScissorEnable = FALSE;
					rDesc.MultisampleEnable = TRUE;
					rDesc.AntialiasedLineEnable = FALSE;
					MainDevice->CreateRasterizerState(&rDesc, &rState);
					DeviceContext->RSSetState(rState);

					D3D11_SAMPLER_DESC SampleDesc;
					SampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					SampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					SampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					SampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					SampleDesc.MinLOD = FLT_MAX;
					SampleDesc.MaxLOD = FLT_MAX;
					SampleDesc.MipLODBias = 0.0f;
					SampleDesc.MaxAnisotropy = 1;
					SampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
					SampleDesc.BorderColor[0] = 0.0f;
					SampleDesc.BorderColor[1] = 0.0f;
					SampleDesc.BorderColor[2] = 0.0f;
					SampleDesc.BorderColor[3] = 0.0f;
					

					MainDevice->CreateSamplerState(&SampleDesc, &SamplerState);
					ID3D11SamplerState* InputSampler[] = { SamplerState };
					DeviceContext->PSSetSamplers(0, 1, InputSampler);

					
					D3D11_BLEND_DESC blendDesc = { 0 };
					blendDesc.AlphaToCoverageEnable = false;
					blendDesc.IndependentBlendEnable = false;


					blendDesc.RenderTarget[0].BlendEnable = true;
					blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; 
					blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
					blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
					blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					
					MainDevice->CreateBlendState(&blendDesc, &BlendState);
					float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
					DeviceContext->OMSetBlendState(BlendState, blendFactor, 0xffffffff);
					//Set Constant buffers (Vertex & Pixel) 
					CreateConstantBuffer(&VertexConstant.data, sizeof(VertexConstant.data), &VertexConstant.buffer);
					SetVConstantBuffer(VertexConstant.buffer);


					CreateConstantBuffer(&PixelConstant.data, sizeof(PixelConstant.data), &PixelConstant.buffer);
					SetPConstantBuffer(PixelConstant.buffer);
				}
			
		}

		int CreateVertexShader(LPCVOID shaderData, UINT dataByteSize, ID3D11VertexShader** targetShader)
		{
			HRESULT hr = MainDevice->CreateVertexShader(shaderData, dataByteSize, NULL, targetShader);
			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		int CreatePixelShader(LPCVOID shaderData, UINT dataByteSize, ID3D11PixelShader** targetShader)
		{
			HRESULT hr = MainDevice->CreatePixelShader(shaderData, dataByteSize, NULL, targetShader);
			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		void SetInputLayout(LPCVOID VertexShaderCode, size_t CodeSize)
		{
			D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
			};

			MainDevice->CreateInputLayout(inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), VertexShaderCode, CodeSize, &InputLayout);
			DeviceContext->IASetInputLayout(InputLayout);
		}
		void SetVertexShader(ID3D11VertexShader* vShader)
		{
			DeviceContext->VSSetShader(vShader, NULL, 0);
		}
		void SetPixelShader(ID3D11PixelShader* pShader)
		{
			DeviceContext->PSSetShader(pShader, NULL, 0);
		}
		int CreateTexture2D(LPCVOID Data, UINT TextureWidth, UINT TextureHeight,ID3D11Texture2D** targetBuffer,DXGI_FORMAT Format = DXGI_FORMAT_R32G32B32A32_FLOAT,UINT BytesPerPixel = 16)
		{
			//Ceate 64bit RGBA texture 
			size_t PixelSize = BytesPerPixel;
			D3D11_TEXTURE2D_DESC TextureDesc = { 0 };
			TextureDesc.Width = TextureWidth;
			TextureDesc.Height = TextureHeight;
			TextureDesc.MipLevels = 1;
			TextureDesc.ArraySize = 1;
			TextureDesc.SampleDesc.Count = 1;
			TextureDesc.Format = Format;
			TextureDesc.Usage = D3D11_USAGE_DEFAULT; //CPU cannot write to texture
			TextureDesc.BindFlags = (D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE);

			D3D11_SUBRESOURCE_DATA SubResourceData = { 0 };
			HRESULT hr ;
			if (Data)
			{
				SubResourceData.pSysMem = Data;
				SubResourceData.SysMemPitch = BytesPerPixel * TextureWidth;
				hr = MainDevice->CreateTexture2D(&TextureDesc, &SubResourceData, targetBuffer);
			}
			else { 
				hr = MainDevice->CreateTexture2D(&TextureDesc, NULL, targetBuffer);
			}
			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		int CreateConstantBuffer(LPCVOID data, UINT dataSize, ID3D11Buffer** targetBuffer)
		{
			UINT size = ((dataSize/16) + 1) * 16;
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = size;
			D3D11_SUBRESOURCE_DATA sResource = { 0 };
			sResource.pSysMem = data;

			HRESULT hr = MainDevice->CreateBuffer(&bufferDesc, &sResource, targetBuffer);

			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		int CreateIndexBuffer(unsigned int* orderArray, UINT dataSize, ID3D11Buffer** targetBuffer)
		{
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.ByteWidth = dataSize;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
			subresourceData.pSysMem = orderArray;
			HRESULT hr = MainDevice->CreateBuffer(&bufferDesc, &subresourceData, targetBuffer);

			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		bool CreateVertexBuffer(LPCVOID data, UINT dataSize, ID3D11Buffer** targetBuffer)
		{
			D3D11_BUFFER_DESC bufferDesc = { 0 };
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = dataSize;
			D3D11_SUBRESOURCE_DATA sResource = { 0 };
			sResource.pSysMem = data;
			HRESULT hr = MainDevice->CreateBuffer(&bufferDesc, &sResource, targetBuffer);

			if (hr == S_OK) { return 1; }
			else { return 0; }
		}
		bool CreateRenderTargetView(ID3D11Resource* InputResource, ID3D11RenderTargetView** TargetView)
		{
			HRESULT HR = MainDevice->CreateRenderTargetView(InputResource, NULL, TargetView);
			if (HR == S_OK) { return 1; }
			else { return 0;  }
		}
		void SetRenderTargetView(ID3D11RenderTargetView* InputView) 
		{
			ID3D11RenderTargetView* RT[] = { InputView };
			DeviceContext->OMSetRenderTargets(1, RT, NULL);
		}
		void SetRenderTargetViewPort(D3D11_VIEWPORT Viewport) 
		{
			D3D11_VIEWPORT VP[] = { Viewport };
			DeviceContext->RSSetViewports(1, VP);
		}
		void ClearRenderTargetView(DirectX::XMFLOAT4 Colour, ID3D11RenderTargetView* RenderTargetView)
		{
			float BGColour[4];
			BGColour[0] = Colour.x;
			BGColour[1] = Colour.y;
			BGColour[2] = Colour.z;
			BGColour[3] = Colour.w;
			DeviceContext->ClearRenderTargetView(RenderTargetView, BGColour);
		}
		bool CreateShaderResourceView(ID3D11Resource* InputResource, ID3D11ShaderResourceView** TargetView)
		{
			HRESULT HR = MainDevice->CreateShaderResourceView(InputResource, NULL, TargetView);
			if (HR == S_OK) { return 1; }
			else { return 0; }
		}
		void SetPixelShaderResource(ID3D11ShaderResourceView* ShaderResource,UINT SlotIndex = 0)
		{
			ID3D11ShaderResourceView* Views[] = { ShaderResource };
			DeviceContext->PSSetShaderResources(SlotIndex, 1, Views);
		}
		void WriteToBuffer(LPCVOID data, size_t dataSize, ID3D11Buffer* buffer) //Write to a buffer (make sure the buffer was created with writing in mind
		{
			D3D11_MAPPED_SUBRESOURCE targetData = { 0 };
			HRESULT hr = DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &targetData);
			if (hr == S_OK) //Manage to get access, start writing
			{
				memcpy(targetData.pData, data, dataSize);
			}
			DeviceContext->Unmap(buffer, 0);
		}
		void SetVConstantBuffer(ID3D11Buffer* inputBuffer)
		{
			ID3D11Buffer* buffer[] = { inputBuffer };
			DeviceContext->VSSetConstantBuffers(0, 1, buffer);
		}
		void SetPConstantBuffer(ID3D11Buffer* inputBuffer)
		{
			ID3D11Buffer* buffer[] = { inputBuffer };
			DeviceContext->PSSetConstantBuffers(0, 1, buffer);
		}
		void SetIndexBuffer(ID3D11Buffer* inputBuffer)
		{
			ID3D11Buffer* buffer = { inputBuffer };
			DeviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
		}
		void SetVertexBuffer(ID3D11Buffer* inptBuffer, UINT objectSize)
		{
			ID3D11Buffer* buffer[] = { inptBuffer };
			UINT stride[] = { objectSize };
			UINT offset[] = { 0 };
			DeviceContext->IASetVertexBuffers(0, 1, buffer, stride, offset);
		}
		void Flush()
		{
			DeviceContext->Flush();
		}
		void Draw(int IndexCount = 3)
		{
			DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			DeviceContext->DrawIndexed(IndexCount, 0, 0);
		}

		struct VConstant
		{
			VConstant() {}
			VConstant(EDX::DirectXDevice DXDevice) {}
			EDX::VertexConstant data;
			CComPtr<ID3D11Buffer> buffer;
			void Update()
			{
				DXDevice.WriteToBuffer(&data, sizeof(data), buffer);
			}
		}VertexConstant;
		struct PConstant
		{
			EDX::PixelConstant data;
			CComPtr<ID3D11Buffer> buffer;
			void Update()
			{
				DXDevice.WriteToBuffer(&data, sizeof(data), buffer);
			}
		}PixelConstant;
		CComPtr<ID3D11Device> MainDevice = nullptr;
		CComPtr<IDXGIFactory2> DXGIFactory = nullptr;
		private:

		CComPtr<ID3D11DeviceContext> DeviceContext = nullptr;
		CComPtr<ID3D11InputLayout> InputLayout = nullptr;
		CComPtr<ID3D11SamplerState> SamplerState;
		CComPtr<ID3D11BlendState> BlendState;

	}DXDevice(true);
	struct Camera2D
	{
		Camera2D()
		{
			ViewMatrix.Translate = DirectX::XMFLOAT3(0, 0, 0);
			ViewMatrix.Rotate = DirectX::XMFLOAT3(0, 0, 0);

			ProjectionMatrix.DepthMinMax.x = 0;
			ProjectionMatrix.DepthMinMax.y = 100;

			ProjectionMatrix.WidthHeight.x = 1;
			ProjectionMatrix.WidthHeight.y = 1;
		}
		EDX::ViewMatrix ViewMatrix;
		EDX::Projection2DMatrix ProjectionMatrix;
		void Set()
		{
			EDX::DXDevice.VertexConstant.data.ViewMatrix = ViewMatrix.ReturnMatrix();
			EDX::DXDevice.VertexConstant.data.ProjectionMatrix = ProjectionMatrix.ReturnMatrix();
			EDX::DXDevice.VertexConstant.Update();
		}
		void SetPosition(DirectX::XMFLOAT3 NewPosition)
		{
			ViewMatrix.Translate.x = -NewPosition.x;
			ViewMatrix.Translate.y = -NewPosition.y;
			ViewMatrix.Translate.z = 0;
		}
		void SetSize(DirectX::XMFLOAT2 WidthHeight)
		{
			if (WidthHeight.x < 1 && WidthHeight.y < 1) return;
			ProjectionMatrix.WidthHeight.x = WidthHeight.x;
			ProjectionMatrix.WidthHeight.y = WidthHeight.y;
		}
		void SetDepth(DirectX::XMFLOAT2 DepthMinMax)
		{
			ProjectionMatrix.DepthMinMax.x = DepthMinMax.x;
			ProjectionMatrix.DepthMinMax.y = DepthMinMax.y;
		}
		void SetAngle(DirectX::XMFLOAT3 NewRotation)
		{
			ViewMatrix.Rotate.x = NewRotation.x;
			ViewMatrix.Rotate.y = NewRotation.y;
			ViewMatrix.Rotate.z = NewRotation.z;
		}
		

	};

	class RenderTargetBase
	{
	public:
		RenderTargetBase() {}
		virtual void SetAsRenderTarget() = 0;
		virtual void Clear(DirectX::XMFLOAT4 Colour) = 0;
		virtual void SetToPixelShader(UINT ResourceSlot = 0) = 0;
		virtual UINT GetWidth() = 0;
		virtual UINT GetHeight() = 0;
	};
	//Window RenderTarget
	struct HWNDRenderTarget : public RenderTargetBase
	{
		HWNDRenderTarget() {}
		HWNDRenderTarget(HWND Hwnd, BOOL AntiAlias = true,BOOL AlphaBlending = true)
		{
			if (!DXDevice.MainDevice) { return; }
			else if (IsWindow(Hwnd))
			{
				RECT WindowClientR = { 0 };
				GetClientRect(Hwnd, &WindowClientR);
				//Create Swapchain
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				swapChainDesc.SampleDesc.Count = 1;

				//Avoid DX Warning for swap chain having size of (0,0)
				if (WindowClientR.right < 10)  { WindowClientR.right = 10;  }
				if (WindowClientR.bottom < 10) { WindowClientR.bottom = 10; }
				swapChainDesc.Width = WindowClientR.right;
				swapChainDesc.Height = WindowClientR.bottom;

				DXDevice.DXGIFactory->CreateSwapChainForHwnd(
					DXDevice.MainDevice,
					Hwnd,
					&swapChainDesc,
					NULL, NULL,
					&SwapChain
				);

				if (SwapChain)
				{
					SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Texture2D);
					DXDevice.MainDevice->CreateRenderTargetView(Texture2D, NULL, &View);

					CComPtr<IDXGIFactory2> PFactory;
					SwapChain->GetParent(__uuidof(IDXGIFactory2),(void**) &PFactory);
					PFactory->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_ALT_ENTER);
					if (View == NULL)
					{
						MessageBox(NULL, L"RenderTargetView (HWND) Failed", L"Error", NULL);
					}
				}
				else { MessageBox(NULL, L"SwapChain Failed", L"Error", NULL); }
			}
			else { MessageBox(NULL, L"Tried to make a HWNDRenderTarget but no valid Window given.", L"Error", NULL); }
		}
		void SetAsRenderTarget()
		{
			EDX::DXDevice.SetRenderTargetView(View);
			D3D11_VIEWPORT VP = {0};
			VP.TopLeftX = 0;
			VP.TopLeftY = 0; 
			VP.Width = GetWidth();
			VP.Height = GetHeight();
			VP.MinDepth = 0;
			VP.MaxDepth = 1;
			EDX::DXDevice.SetRenderTargetViewPort(VP);
			EDX::Camera2D Camera;
			Camera.SetSize(DirectX::XMFLOAT2(GetWidth(), GetHeight()));
			Camera.SetDepth(DirectX::XMFLOAT2(0, 100));
			Camera.Set();
		}
		void SetToPixelShader(UINT ResourceSlot = 0)
		{

		}
		void ResizeBuffer(int Width, int Height)
		{
			if (SwapChain && View && Texture2D)
			{
				HRESULT HR;
				View.Release();
				Texture2D.Release();
				HR = SwapChain->ResizeBuffers(2, Width, Height, DXGI_FORMAT_UNKNOWN, NULL);
				HR = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Texture2D);
				if (HR == S_OK)
				{
					DXDevice.MainDevice->CreateRenderTargetView(Texture2D, NULL, &View);
				}
			}
		}
		void ResizeBuffer()
		{
			if (SwapChain)
			{
				HWND hwnd;
				SwapChain->GetHwnd(&hwnd);
				RECT rect;
				GetClientRect(hwnd, &rect);
				if (rect.right > 0 && rect.bottom > 0)
				{
					ResizeBuffer(rect.right, rect.bottom);
				}
			}
		}
		void ResizeWindow(int Width, int Height)
		{
			DXGI_MODE_DESC ModeDesc = { 0 };
			ModeDesc.Width = Width;
			ModeDesc.Height = Height;
			SwapChain->ResizeTarget(&ModeDesc);
		}
		void ToggleFullScreen(bool FullScreen)
		{
			SwapChain->SetFullscreenState(FullScreen, NULL);
			RECT ClientRect;
			HWND Hwnd;
			SwapChain->GetHwnd(&Hwnd);
			GetClientRect(Hwnd, &ClientRect);
			ResizeBuffer(ClientRect.right * 2, ClientRect.bottom * 2);
		}
		void Clear(DirectX::XMFLOAT4 Colour = DirectX::XMFLOAT4(1,1,1,1))
		{
			if (View)
			{
				EDX::DXDevice.ClearRenderTargetView(Colour, View);
			}
		}
		UINT GetWidth()
		{
			if (SwapChain)
			{
				DXGI_SWAP_CHAIN_DESC desc;
				SwapChain->GetDesc(&desc);
				return desc.BufferDesc.Width;
			}
			else { return 0; }
		}
		UINT GetHeight()
		{
			if (SwapChain)
			{
				DXGI_SWAP_CHAIN_DESC desc;
				SwapChain->GetDesc(&desc);
				return desc.BufferDesc.Height;
			}
			else { return 0; }
		}
		void Flip(int Vsync = 0)
		{
			if (Vsync > 4) Vsync = 4;
			if (SwapChain) SwapChain->Present(Vsync, 0);
		}
		CComPtr<IDXGISwapChain1> SwapChain;
		CComPtr<ID3D11Texture2D> Texture2D;
		CComPtr<ID3D11RenderTargetView> View;
		operator bool()
		{
			return View;
		}
	};
}
