#pragma once
#pragma pack(4)
/*
Defines useful structures to be used with 
*/
#include <MyDirectX.h>
#include <MyDirectXImage.h>
#include <DirectXMath.h>
#include <d3d11.h>

#include <math.h>




namespace EDX
{
	class Model
	{
	public:
		Model() {}
		void AddTri(EDX::Vertex Points[3])
		{
			int VertexCount = Input.Vertecies.size();
			for (int i = 0; i < 3; i++)
			{
				Input.Vertecies.push_back(Points[i]);
				Input.Indecies.push_back(i + VertexCount);
			}
		}
		void AddQuad(EDX::Vertex Points[4])
		{
			int VertexCount = Input.Vertecies.size();
			for (int i = 0; i < 4; i++)
			{
				Input.Vertecies.push_back(Points[i]);
			}

			unsigned int Index[6] = { 0,1,2,1,3,2 };

			for (int i = 0; i < 6; i++)
			{
				Index[i] += VertexCount;
				Input.Indecies.push_back(Index[i]);
			}
		}
		void Create()
		{
			//Finalise the model
			if (VertexBuffer) { VertexBuffer.Release(); }
			if (IndexBuffer) { IndexBuffer.Release();  }

			EDX::Vertex* Vertecies = new EDX::Vertex[Input.Vertecies.size()];
			unsigned int* Indecies = new unsigned int[Input.Indecies.size()];

			for (int i = 0; i < Input.Vertecies.size(); i++)
			{
				Vertecies[i] = Input.Vertecies.at(i);
			}

			for (int i = 0; i < Input.Indecies.size(); i++)
			{
				Indecies[i] = Input.Indecies.at(i);
			}

			IndexCount = Input.Indecies.size();
			EDX::DXDevice.CreateVertexBuffer(Vertecies, sizeof(EDX::Vertex) * Input.Vertecies.size(), &VertexBuffer);
			EDX::DXDevice.CreateIndexBuffer(Indecies, sizeof(unsigned int) * Input.Indecies.size(), &IndexBuffer);


			Input.Vertecies.clear();
			Input.Indecies.clear();

			delete[] Vertecies;
			delete[] Indecies;

			Input.Texture.ImageH.clear();
			Input.Texture.ImageW.clear();
		}
		int GetIndexCount()
		{
			return IndexCount;
		}
		void SetColourBalance(DirectX::XMFLOAT4 Balance)
		{
			PConstant.ColourBalance = Balance; 
		}
		void SetTexturing(bool Texturing)
		{
			PConstant.Texturing = DirectX::XMFLOAT2(Texturing, Texturing);
		}
		void SetAlphaMasking(bool Masking)
		{
			PConstant.AlphaMasking = DirectX::XMFLOAT2(Masking, Masking);
		}
		void Draw(
			DirectX::XMFLOAT3 Move = DirectX::XMFLOAT3(0, 0, 0), 
			DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3(1, 1, 1), 
			DirectX::XMFLOAT3 Rotate = DirectX::XMFLOAT3(0, 0, 0))
		{
			EDX::WorldMatrix WorldMatrix;

			WorldMatrix.Rotate = Rotate;
			WorldMatrix.Translate = Move;
			WorldMatrix.Scale = Scale;

			EDX::DXDevice.VertexConstant.data.WorldMatrix = WorldMatrix.ReturnMatrix();
			EDX::DXDevice.VertexConstant.Update();
			EDX::DXDevice.PixelConstant.data = PConstant;
			EDX::DXDevice.PixelConstant.Update();

			EDX::DXDevice.SetIndexBuffer(IndexBuffer);
			EDX::DXDevice.SetVertexBuffer(VertexBuffer, sizeof(Vertex));
			EDX::DXDevice.Draw(IndexCount);
		}
	private:
		struct Input
		{
			std::vector<EDX::Vertex> Vertecies;
			std::vector<unsigned int> Indecies;
			struct Texture
			{
				std::vector<UINT> ImageW;
				std::vector<UINT> ImageH;
			}Texture;
		}Input;
		CComPtr<ID3D11Buffer> VertexBuffer;
		CComPtr<ID3D11Buffer> IndexBuffer;
		int IndexCount;
		struct Texture
		{
			std::vector<EDX::Texture2D> Frame;
			int FrameIndex = 0;
		}Texture;
		EDX::PixelConstant PConstant;
		
	};
}

namespace EDX
{
	namespace Shape2D
	{
		class Rect
		{
		public:
			Rect() 
			{
				EDX::Vertex V[4];
				V[0].position = DirectX::XMFLOAT3(-1,  1, 0);
				V[1].position = DirectX::XMFLOAT3(1,  1, 0);
				V[2].position = DirectX::XMFLOAT3(-1, -1, 0);
				V[3].position = DirectX::XMFLOAT3(1, -1, 0);


				for (int i = 0; i < 4; i++)
				{
					V[i].colour = DirectX::XMFLOAT4(1, 1, 1, 1);
				}
				OutputModel.AddQuad(V);
				OutputModel.Create();
			}
			void Draw(DirectX::XMFLOAT3 StartPoint, DirectX::XMFLOAT2 Size, DirectX::XMFLOAT4 Colour, float angle = 0)
			{
				OutputModel.SetColourBalance(Colour);
				float Width = Size.x;
				float Height = Size.y;
				OutputModel.Draw(StartPoint, DirectX::XMFLOAT3(Width, Height, 1), DirectX::XMFLOAT3(0, 0, angle));

			}
		private:
			EDX::Model OutputModel;
		}Rect;
		class Sprite
		{
		public:
			Sprite() 
			{
				EDX::Vertex V[4];
				V[0].position = DirectX::XMFLOAT3(-1, -1, 0);
				V[1].position = DirectX::XMFLOAT3(1, -1, 0);
				V[2].position = DirectX::XMFLOAT3(-1, 1, 0);
				V[3].position = DirectX::XMFLOAT3(1, 1, 0);

				V[0].TexCoord = DirectX::XMFLOAT2(0, 1);
				V[1].TexCoord = DirectX::XMFLOAT2(1, 1);
				V[2].TexCoord = DirectX::XMFLOAT2(0, 0);
				V[3].TexCoord = DirectX::XMFLOAT2(1, 0);

				OutputModel.AddQuad(V);
				OutputModel.SetTexturing(true);
				OutputModel.Create();
			}
			void Draw(DirectX::XMFLOAT3 CentrePoint,
				DirectX::XMFLOAT2 Size, float Angle, 
				Texture2D InputTex)
			{
				InputTex.SetToPixelShader(0);
				OutputModel.Draw(
					CentrePoint,
					DirectX::XMFLOAT3(Size.x, Size.y, 1),
					DirectX::XMFLOAT3(0, 0,Angle)
				);
			}
		private:
			EDX::Model OutputModel;
		}Sprite;
		class Line
		{
		public:
			Line()
			{
				EDX::Vertex V[4];
				V[0].position = DirectX::XMFLOAT3(0,-1, 0);
				V[1].position = DirectX::XMFLOAT3(1, -1, 0);
				V[2].position = DirectX::XMFLOAT3(0, 1, 0);
				V[3].position = DirectX::XMFLOAT3(1, 1, 0);

				for (int i = 0; i < 4; i++)
				{
					V[i].colour = DirectX::XMFLOAT4(1, 1, 1, 1);
				}

				OutputModel.AddQuad(V);
				OutputModel.Create();
			}
			void Draw(DirectX::XMFLOAT3 StartPoint, DirectX::XMFLOAT3 EndPoint,float Width, DirectX::XMFLOAT4 Colour)
			{
				std::wstring debugS;
				float xChange = EndPoint.x - StartPoint.x;
				float yChange = EndPoint.y - StartPoint.y;
				float Angle = (float)atan2(yChange , xChange);
				float Length = sqrt(pow(xChange, 2) + pow(yChange, 2));

				DirectX::XMFLOAT3 DrawPoint = StartPoint;
				DrawPoint.x += Length/2;

				OutputModel.SetColourBalance(Colour);
				OutputModel.Draw(
					StartPoint, DirectX::XMFLOAT3(Length, Width, 1),
					DirectX::XMFLOAT3(0,0,Angle)
				);
			}
		private:
			EDX::Model OutputModel;
		}Line;
		class Circle 
		{
		public:
			Circle() 
			{
				UINT Sides = 360;
				float Div = (float)360/Sides;
				for (int i = 0; i <= Sides; i++)
				{
					float Radian[2] = { 0 };
					for (int a = 0; a < 2; a++)
					{
						float angle = Div * (i + a);
						Radian[a] = angle * (3.14 / 180);
					}

					EDX::Vertex vPoint[3];
					vPoint[0].position = DirectX::XMFLOAT3(0, 0, 1);
					vPoint[1].position = DirectX::XMFLOAT3(cos(Radian[0]), sin(Radian[0]), 1);
					vPoint[2].position = DirectX::XMFLOAT3(cos(Radian[1]), sin(Radian[1]), 1);

					for (int v = 0; v < 3; v++)
					{
						vPoint[v].colour = DirectX::XMFLOAT4(1, 1, 1, 1);
					}

					OutputModel.AddTri(vPoint);

				}
				OutputModel.Create();

			}
			void Draw(DirectX::XMFLOAT3 StartPoint, float Radius, DirectX::XMFLOAT4 Colour)
			{
				OutputModel.SetColourBalance(Colour);
				OutputModel.Draw(
					StartPoint,
					DirectX::XMFLOAT3(Radius, Radius, 1)
				);

			}
		private:
			EDX::Model OutputModel;
		}Cirlce;
	}
}
