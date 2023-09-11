#include <Windows.h>
#include <CommCtrl.h>
#include <MyGame.h>



LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    WNDCLASS WindowClass = {0};
    WindowClass.lpfnWndProc = &MainWindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
    WindowClass.lpszClassName = L"Main Window Class";
    if (!RegisterClass(&WindowClass)) { return 0; }

    RECT createRect = { 0 } ;
    createRect.right = 500;
    createRect.bottom = 200;

    EScene::DXWindow MainWindow = EScene::DXWindow(WindowClass, L"", (WS_OVERLAPPEDWINDOW|WS_VISIBLE));

    if (!MainWindow)
    {
        return 0;
    }
    else
    {

        EResource PSResource = EResource(107, L"SHADER");
        CComPtr<ID3D11PixelShader> PShader;
        EDX::DXDevice.CreatePixelShader(PSResource.GetDataPointer(), PSResource.GetDataSize(), &PShader);
        EDX::DXDevice.SetPixelShader(PShader);


        EResource VSResource = EResource(106, L"SHADER");
        CComPtr<ID3D11VertexShader> VShader;
        EDX::DXDevice.CreateVertexShader(VSResource.GetDataPointer(), VSResource.GetDataSize(), &VShader);
        EDX::DXDevice.SetVertexShader(VShader);
        EDX::DXDevice.SetInputLayout(VSResource.GetDataPointer(), VSResource.GetDataSize());
        


        EResource FontResource = EResource();
        FontResource.Load(109,L"CFONT");
        EDX::Text::Font TestFont(FontResource);

        while (MainWindow.GetMSG())
        {
            
            MainWindow.ReSizeBuffer();
            MainWindow.SetAsRenderTarget();
            MainWindow.Clear(DirectX::XMFLOAT4(0, 0, 0, 0));
            EDX::Text::String TestString(L"Text in DirectX");
            TestString.SetXPositioning(EDX_TEXT_CENTRE);
            TestFont.Draw(TestString, 30, DirectX::XMFLOAT3(0, 0, 0));
            MainWindow.Flip();
        }
    }

    return 1;
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        return 0;
    }
    return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

