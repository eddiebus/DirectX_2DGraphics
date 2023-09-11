#pragma once
#pragma comment(lib,"freetype.lib")

#include <MyDirectX.h>
#include <MyDirectXshape.h>
#include <MYDirectXImage.h>
#include <ResourceLoading.h>


#define EDX_TEXT_CENTRE 0U
#define EDX_TEXT_RIGHT 1U
#define EDX_TEXT_LEFT 2U

//Freetype (Text)
#include <Text/freetype-2.10.0/include/ft2build.h>
#include <freetype/ftmodapi.h>
#include FT_FREETYPE_H

namespace EDX
{

	namespace Text
	{
		class Library
		{
		public:
			Library()
			{
				FT_Init_FreeType(&FTLibrary);
			}
			~Library()
			{
				FT_Done_FreeType(FTLibrary);
			}
			FT_Library Get()
			{
				return FTLibrary;
			}
		private:
			FT_Library FTLibrary;
		}Library;
	}

	namespace Text
	{
		class Char 
		{
		public:
			Char() {}
			Char(FT_Face InputFont,UINT glyphIndex,UINT pSize = 32)
			{
				GlyphIndex = glyphIndex;
				PixelSize = pSize;
				FT_Set_Pixel_Sizes(InputFont, PixelSize, PixelSize);
				FT_Load_Glyph(InputFont, GlyphIndex, FT_LOAD_DEFAULT);

				FT_ULong height = InputFont->glyph->metrics.height;
				FT_ULong width = InputFont->glyph->metrics.width;
				FT_ULong YBearing = InputFont->glyph->metrics.horiBearingY;


				FT_Render_Glyph(InputFont->glyph, FT_RENDER_MODE_NORMAL);

				BitmapWidth = InputFont->glyph->bitmap.width;
				BitmapHeight = InputFont->glyph->bitmap.rows;
				UINT BitmapPitch = InputFont->glyph->bitmap.pitch;
				UINT BitmapArea = BitmapWidth * BitmapHeight;

				if (BitmapWidth && BitmapHeight)
				{
					AlphaMask = Texture2D(InputFont->glyph->bitmap.buffer, BitmapWidth, BitmapHeight, DXGI_FORMAT_R8_UNORM, 1);
				}

				EDX::Vertex Vertecies[4];
				Vertecies[0].position = DirectX::XMFLOAT3(0, BitmapHeight, 0);
				Vertecies[1].position = DirectX::XMFLOAT3(BitmapWidth, BitmapHeight, 0);
				Vertecies[2].position = DirectX::XMFLOAT3(0, 0, 0);
				Vertecies[3].position = DirectX::XMFLOAT3(BitmapWidth, 0, 0);

				for (int i = 0; i < 4; i++)
				{
					Vertecies[i].colour = DirectX::XMFLOAT4(1, 1, 1, 1);
					Vertecies[i].position.y -= BitmapHeight - (BitmapHeight);
				}
				Vertecies[0].TexCoord = DirectX::XMFLOAT2(0, 0);
				Vertecies[1].TexCoord = DirectX::XMFLOAT2(1, 0);
				Vertecies[2].TexCoord = DirectX::XMFLOAT2(0, 1);
				Vertecies[3].TexCoord = DirectX::XMFLOAT2(1, 1);

				OutputModel.AddQuad(Vertecies);
				OutputModel.SetAlphaMasking(true);
				OutputModel.Create();

			}

			void Draw(float Height, DirectX::XMFLOAT3 DrawPoint, DirectX::XMFLOAT4 Colour = DirectX::XMFLOAT4(1,1,1,1))
			{
				OutputModel.SetColourBalance(Colour);
				float scaleFactor = Height;
				scaleFactor /= PixelSize;
				AlphaMask.SetToPixelShader(1);
				OutputModel.Draw(
					DrawPoint,
					DirectX::XMFLOAT3(scaleFactor, scaleFactor, 1.0f)
				);

			}
			UINT GetPixelSize()
			{
				return PixelSize;
			}
			UINT GetBitmapHeight()
			{
				return BitmapHeight;
			}
			UINT GetBitmapWidth()
			{
				return BitmapWidth;
			}
			operator bool()
			{
				return AlphaMask;
			}
		private:
			Texture2D AlphaMask;
			UINT GlyphIndex;
			UINT BitmapHeight;
			UINT BitmapWidth;
			UINT PixelSize;
			EDX::Model OutputModel;
		};
		
		/*
		* Boxing Restrictions For Strings
		* 0 Values mean no restriction on text size
		*/
		struct TextBox
		{
			TextBox() {}
			TextBox(float w, float h)
			{
				Width = w;
				Height = h;
			}
			float Width = 0;
			float Height = 0;
		};

		/*
		* Holds Information used by Font class for advance text drawing
		*/
		class String
		{
		public:
			String() {}
			String(std::wstring InputString)
			{
				OutputString = InputString;
				for (int i = 0; i < OutputString.size(); i++)
				{
					CharColour.push_back(DirectX::XMFLOAT4(1, 1, 1, 1));
				}
			}
			std::wstring GetString() { return OutputString; }

			void SetXPositioning(UINT NewP){xPositioning = NewP;}
			UINT GetXPositioning() { return  xPositioning; }
			void SetYPositioning(UINT NewP) { yPositioning = NewP; }
			UINT GetYPositioning() { return  yPositioning; }

			void SetTextBox(TextBox inputBox)
			{
				textBox = inputBox;
			}

			TextBox GetTextBox() { return textBox; }

			void SetLineAdvance(float LOffset) { LineAdvance = LOffset; }
			float GetLineAdvance() { return LineAdvance; }

			void SetCharSpacing(float SOffset) { CharSpacing = SOffset; }
			float GetCharSpacing() { return CharSpacing; }
		private:
			std::wstring OutputString;
			std::vector<DirectX::XMFLOAT4> CharColour;
			TextBox textBox;
			UINT xPositioning = EDX_TEXT_RIGHT;
			UINT yPositioning = EDX_TEXT_RIGHT;
			float LineAdvance = 0;
			float CharSpacing = 0;
		};

		/*
		Collection of EDX::Char that makes a font.
		Can draw basic or EDX::String to RenderTarget.
		*/
		class Font
		{
		public:
			Font()
			{
			}
			Font(EResource Resource)
			{

				if (!Resource.GetDataSize())return;

				FT_Error error = FT_New_Memory_Face(
					Text::Library.Get(),
					(FT_Byte*)Resource.GetDataPointer(),
					(FT_Long)Resource.GetDataSize(),
					0,
					&FontFace
				);

				FT_Set_Pixel_Sizes(
					FontFace,
					PixelSize, PixelSize
				);

				for (int i = 0; i < FontFace->num_glyphs; i++)
				{
					Character.push_back(Char(FontFace, i,PixelSize));
				}
			}
			UINT GetGlyphCount()
			{
				return FontFace->num_glyphs;
			}
			UINT GetPixelSize()
			{
				return PixelSize;
			}
			//Basic String Drawing 
			void Draw(std::wstring InputString, float Height, DirectX::XMFLOAT3 StartPoint, DirectX::XMFLOAT4 Colour)
			{
				float ScaleFactor = Height / PixelSize;
				DirectX::XMFLOAT3 PenPoint = StartPoint;
				for (int c = 0; c < InputString.size(); c++)
				{
					wchar_t indexChar = InputString.at(c);
					switch (indexChar)
					{
					case ' ':
						PenPoint.x += Height/10;
						break;
					case '\n':
						PenPoint.x = StartPoint.x;
						PenPoint.y -= Height * 1.5;
						break;
					default:
						break;
					}
					UINT GlyphIndex = FT_Get_Char_Index(FontFace, indexChar);
					if (GlyphIndex)
					{ 
						Character.at(GlyphIndex).Draw(Height, DirectX::XMFLOAT3(PenPoint.x,PenPoint.y,StartPoint.z),Colour);
					}
					PenPoint.x += Character.at(GlyphIndex).GetBitmapWidth() * ScaleFactor;
					PenPoint.x += 2;
				}
			}
			
			/*
			Advance String Drawing
			Can have X and Y allignment
			Can have textBox fixing
			Creates a series of draw points before drawing
			*/
			void Draw(EDX::Text::String InputString, float Height, DirectX::XMFLOAT3 StartPoint)
			{
				std::wstring string = InputString.GetString(); 
				//Scale factor from font to pizelSize

				float ScaleFactor = Height / PixelSize;
				DirectX::XMFLOAT3 PenPoint = StartPoint;

				float TotalHeight = 0; //Used for Y Positioning

				//Line Split Info
				std::vector<std::wstring> LineString; 
				std::vector<float> LineWidth; //Width of each line
				//2D vector of line drawpoints
				//1st Vector is the line
				//2nd Each draw point

				std::vector<std::vector<DirectX::XMFLOAT3>> LineDrawPoint;

				//Line Splits
				std::vector<std::wstring> stringSplit;
				std::vector<std::vector<float>> stringSplitCharSpace; //How much space the line takes
				std::vector<float> stringSplitHeight;

				//Temp Variables to input to splits
				std::wstring split;
				std::vector<float> splitCharSpace;
				float splitHeight = 0;

				//Start reading string and creating splits
				for (int readIndex = 0; readIndex < string.size(); readIndex++)
				{
					wchar_t wCharacter = string.at(readIndex);
					int glyphIndex = FT_Get_Char_Index(FontFace, wCharacter);
					
					split += wCharacter;

					bool addsplit = false;
					float charspace = 0;
					switch (wCharacter)
					{
					case '.':
						addsplit = true;
					case '?':
						addsplit = true;
					case ' ':
						charspace += (Height / 10);
						addsplit = true;
					case '\n':
						addsplit = true;
					default:
						break;
					}

					if (glyphIndex)
					{
						charspace += Character.at(glyphIndex).GetBitmapWidth() * ScaleFactor
							+ InputString.GetCharSpacing() + 1;
						float charHeight = Character.at(glyphIndex).GetBitmapHeight() * ScaleFactor;
						if (charHeight > splitHeight) splitHeight = charHeight;
					}

					splitCharSpace.push_back(charspace);

					if (addsplit || (readIndex+1) == string.size())
					{
						stringSplit.push_back(split);
						stringSplitCharSpace.push_back(splitCharSpace);
						stringSplitHeight.push_back(splitHeight);

						split.clear();
						splitCharSpace.clear();
						splitHeight = 0;
					}
				}

				bool newLine = false; //When true a line will be pushed onto vector
				float remainLineSpace = InputString.GetTextBox().Width; //Space left on the line
				std::wstring tempLineString;
				float tempLineHeight = 0;
				float tempLineWidth = 0;
				std::vector<DirectX::XMFLOAT3> tempDrawPoint;
				for (int splitI = 0; splitI < stringSplit.size(); splitI++)
				{
					std::wstring split = stringSplit.at(splitI);
					std::vector<float> splitCharSpace = stringSplitCharSpace.at(splitI);
					float splitWidth = 0; 

					for (int i = 0; i < splitCharSpace.size(); i++)
						splitWidth += splitCharSpace[i];


					if (std::abs(PenPoint.y - StartPoint.y) > InputString.GetTextBox().Height
						&& InputString.GetTextBox().Height)
						break;

					//There's enough space for this split to fit
					if (!InputString.GetTextBox().Width || splitWidth <= remainLineSpace)
					{
						if (split.at(split.size() - 1) == '\n')newLine = true;
						for (int i = 0; i < split.size();i++)
						{
							tempLineString += split[i];
							tempLineWidth += splitCharSpace[i];
							tempDrawPoint.push_back(PenPoint);
							PenPoint.x += splitCharSpace.at(i);
						}
						remainLineSpace -= splitWidth;

						if (stringSplitHeight.at(splitI) > tempLineHeight) tempLineHeight = stringSplitHeight.at(splitI);
					}
					//There's not enough space for this split to fit
					else if (InputString.GetTextBox().Width && splitWidth > remainLineSpace)
					{
						if (splitWidth <= InputString.GetTextBox().Width) splitI--;
						newLine = true;
					}

					//Add new line
					//Or if last string force new line
					if (newLine || (splitI+1) == stringSplit.size())
					{
						PenPoint.x = StartPoint.x;
						PenPoint.y -= Height;

						LineString.push_back(tempLineString);
						LineWidth.push_back(tempLineWidth);
						LineDrawPoint.push_back(tempDrawPoint);
						TotalHeight += tempLineHeight;

						tempLineString.clear();
						tempDrawPoint.clear();
						tempLineWidth = 0;
						tempLineHeight = 0;
						remainLineSpace = InputString.GetTextBox().Width;
						newLine = false;
					}

				}

				//X  and Y Position
				UINT xPositioning = InputString.GetXPositioning();
				UINT yPositioning = InputString.GetYPositioning();
				for (int lineIndex = 0; lineIndex < LineString.size(); lineIndex++)
				{
					for (int i = 0; i < LineDrawPoint.at(lineIndex).size(); i++)
					{
						float lineW = LineWidth.at(lineIndex);
						switch (xPositioning)
						{
						case EDX_TEXT_CENTRE:
							LineDrawPoint.at(lineIndex).at(i).x -= lineW / 2;
							
							break;
						case EDX_TEXT_LEFT:
							LineDrawPoint.at(lineIndex).at(i).x -= lineW;
							LineDrawPoint.at(lineIndex).at(i).y += TotalHeight;
							break;
						}

						switch (yPositioning)
						{
						case EDX_TEXT_CENTRE:
							LineDrawPoint.at(lineIndex).at(i).y += TotalHeight / 2;
							break;
						case EDX_TEXT_LEFT:
							LineDrawPoint.at(lineIndex).at(i).y += TotalHeight;
						}
					}
				}

				//DrawPoints finished. Draw the result
				for (int lineIndex = 0; lineIndex < LineString.size(); lineIndex++)
				{
					for (int lineChar = 0; lineChar < LineString[lineIndex].size(); lineChar++)
					{
						int glyphIndex = FT_Get_Char_Index(FontFace, LineString.at(lineIndex).at(lineChar));

						if (glyphIndex)
							Character.at(glyphIndex).Draw(
							Height, LineDrawPoint.at(lineIndex).at(lineChar)
						);
					}
				}
			}

			void operator= (Font OtherFont)
			{
				FT_Reference_Face(OtherFont.FontFace);
			}
			~Font()
			{
				FT_Done_Face(FontFace);
			}
		private:
			FT_Face FontFace;
			UINT PixelSize = 128;
			std::vector<Char> Character;
		};

	}

}



