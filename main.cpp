#include <Windows.h>
#include <cmath>


const int WIDTH = 300;
const int HEIGHT = 300;
float v[3][3], v4;
float rho = 300.0f, thetta = 75.0f, phi = 30.0f;
float screenDist = 500.0f;
float A, B, C, D, An, Bn, Cn, Al, Bl, Cl;
float xt[3], yt[3], zt[3];
float alpha;
float th, ph, costh, cosph, sinth, sinph;
float fact = atan(1.0f) / 45.0f;

PAINTSTRUCT ps;
static HBRUSH hBrush;

struct TFPoint {
	float x;
	float y;
	float z;
};

TFPoint vertexes[] = {
	{ -50.0f, -50.0f, -50.0f },
	{ 50.0f,  -50.0f, -50.0f },
	{ 50.0f,  50.0f,  -50.0f },
	{ -50.0f, 50.0f,  -50.0f },
	{ -50.0f, 50.0f,  50.0f },
	{ -50.0f, -50.0f, 50.0f },
	{ 50.0f,  -50.0f, 50.0f },
	{ 50.0f,  50.0f,  50.0f }
};

int faces[6][4] = {
	{ 0, 3, 4, 5 },
	{ 0, 5, 6, 1 },
	{ 2, 7, 4, 3 },
	{ 7, 6, 5, 4 },
	{ 0, 1, 2, 3 },
	{ 2, 1, 6, 7 },
};

void ObserveMatrix(float rho, float thetta, float phi) {
	th = thetta * fact; ph = phi * fact;
	costh = cos(th); sinth = sin(th); cosph = cos(ph); sinph = sin(ph);

	v[0][0] = -sinth; v[0][1] = -cosph * costh; v[0][2] = -sinph * costh;
	v[1][0] = costh; v[1][1] = -cosph * sinth; v[1][2] = -sinph * sinth;
	v[2][0] = 0.0f; v[2][1] = sinph; v[2][2] = -cosph;
	v4 = rho;
}

POINT Perspective(float x, float y, float z) {
	POINT point;
	float xe, ye, ze;
	ObserveMatrix(rho, thetta, phi);
	xe = v[0][0] * x + v[1][0] * y;
	ye = v[0][1] * x + v[1][1] * y + v[2][1] * z;
	ze = v[0][2] * x + v[1][2] * y + v[2][2] * z + v4;

	point.x = screenDist * xe / ze + 400.0f;
	point.y = screenDist * ye / ze + 300.0f;

	return point;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT points[4];
    HDC hdc;
    int sx, sy, xPos, yPos, zDelta;

    switch (message)
    {
    case WM_MOUSEMOVE:
        sx = LOWORD(lParam);
        sy = HIWORD(lParam);
        thetta += ((sx % 180) - 90) / 10.0f;
        phi += ((sy % 180) - 90) / 10.0f;
        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_MOUSEWHEEL:
        zDelta = (int)wParam;
        screenDist += zDelta / 1000000.0f;
        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_PAINT:
    {
        HDC hdc = BeginPaint(hWnd, &ps);

        th = thetta * fact; ph = phi * fact;
        costh = cos(th); sinth = sin(th); cosph = cos(ph); sinph = sin(ph);

        A = rho * sinph * costh; B = rho * sinph * sinth; C = rho * cosph;

        Al = A / (sqrt(A * A + B * B + C * C));
        Bl = B / (sqrt(A * A + B * B + C * C));
        Cl = C / (sqrt(A * A + B * B + C * C));

        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 3; j++) {
                xt[j] = vertexes[faces[i][j]].x;
                yt[j] = vertexes[faces[i][j]].y;
                zt[j] = vertexes[faces[i][j]].z;
            }

            A = yt[0] * (zt[1] - zt[2]) - yt[1] * (zt[0] - zt[2]) + yt[2] * (zt[0] - zt[1]);
            B = -(xt[0] * (zt[1] - zt[2]) - xt[1] * (zt[0] - zt[2]) + xt[2] * (zt[0] - zt[1]));
            A = xt[0] * (yt[1] - yt[2]) - xt[1] * (yt[0] - yt[2]) + xt[2] * (yt[0] - yt[1]);

            An = A / (sqrt(A * A + B * B + C * C));
            Bn = B / (sqrt(A * A + B * B + C * C));
            Cn = C / (sqrt(A * A + B * B + C * C));

            alpha = (An * Al + Bn * Bl + Cn * Cl);

            for (int j = 0; j < 4; j++) {
                points[j] = Perspective(
                    vertexes[faces[i][j]].x,
                    vertexes[faces[i][j]].y,
                    vertexes[faces[i][j]].z
                );
            }

            D = points[0].x * (points[1].y - points[2].y) -
                points[1].x * (points[0].y - points[2].y) +
                points[2].x * (points[0].y - points[1].y);

            if (D < 0) {
                hBrush = CreateSolidBrush(RGB(
                    (1 - alpha) * 255,
                    (1 - alpha) * 255,
                    (1 - alpha) * 255
                ));
                SelectObject(hdc, hBrush);
                Polygon(hdc, points, 4);
            }
        }


        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        DeleteObject(hBrush);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	WNDCLASS wc;

	wc.hInstance = hInstance;
	wc.lpszClassName = L"Window";
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);

	if (!RegisterClass(&wc)) return 0;

	hWnd = CreateWindow(
		L"Window",
		L"Simple real-time clock",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}