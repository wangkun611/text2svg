// text2svg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "getoptPlusPlus/getoptpp.h"
using namespace vlofgren;

typedef PODParameter<wstring> WStringParameter;

int _tmain(int argc, const char* argv[])
{
    // Create a parser
    OptionsParser optp("");
    ParameterSet& ps = optp.getParameters();

    /* An alternative option is to simply extend the options parser and set all this up
    * in the constructor.
    */
    ps.add<StringParameter>('o', "output", "set out file path");
    // Parse argv
    try {
        optp.parse(argc, argv);
    }
    catch (Parameter::ParameterRejected &p){
        // This will happen if the user has fed some malformed parameter to the program
        cerr << p.what() << endl;
        optp.usage();
        return EXIT_FAILURE;
    }
    catch (runtime_error &e) {
        // This will happen if you try to access a parameter that hasn't been set
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    HDC hdc = CreateCompatibleDC(nullptr);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
    SetMapMode(hdc, MM_TEXT);

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -MulDiv(21, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    lf.lfWeight = FW_BOLD;
    lf.lfQuality = ANTIALIASED_QUALITY;
    _tcsncpy(lf.lfFaceName, _T("黑体"), LF_FACESIZE);
    HFONT hFont = CreateFontIndirect(&lf);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    LPCTSTR pszText = _T("比这种赛");
    SIZE extent;
    GetTextExtentPoint32(hdc, pszText, _tcslen(pszText), &extent);

    BeginPath(hdc);
    TextOut(hdc, 0, 0, pszText, _tcslen(pszText));
    CloseFigure(hdc);
    EndPath(hdc);

    int pathPoints = GetPath(hdc, nullptr, nullptr, 0);
    BYTE* pPathTypes = (BYTE*)malloc(sizeof(BYTE) * pathPoints);
    POINT* pPathPoints = (POINT*)malloc(sizeof(POINT) * pathPoints);

    pathPoints == GetPath(hdc, pPathPoints, pPathTypes, pathPoints);

    DeleteObject(hFont);
    DeleteDC(hdc);

    FILE* file = fopen(ps['o'].get<string>().c_str(), "wb");
    if (file) {
        fprintf(file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        fprintf(file, "<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n", extent.cx, extent.cy);
        fprintf(file, "<g>\n<path d=\"");

        for (int i = 0; i < pathPoints; ++i) {
            BYTE t = pPathTypes[i] & ~PT_CLOSEFIGURE;

            switch(t) {
            case PT_MOVETO:
                fprintf(file, "M %d %d \n", pPathPoints[i].x, pPathPoints[i].y);
                break;
            case PT_LINETO:
                fprintf(file, "L %d %d \n", pPathPoints[i].x, pPathPoints[i].y);
                break;
            case PT_BEZIERTO:
            {
                fprintf(file, "C %d %d %d %d %d %d \n", pPathPoints[i].x, pPathPoints[i].y,
                    pPathPoints[i+1].x, pPathPoints[i+1].y,
                    pPathPoints[i+2].x, pPathPoints[i+2].y);
                i += 2;
            }
                break;
            case PT_CLOSEFIGURE:
                fprintf(file, "Z \n");
                break;
            default:
                fprintf(file, "UNKNOWN\n");
                break;
            }
            if ((pPathTypes[i] & PT_CLOSEFIGURE) == PT_CLOSEFIGURE) {
                fprintf(file, "Z \n");
            }
        }

        fprintf(file, "\" /></g>");
        fprintf(file, "</svg>");
        fclose(file);
    }
    return 0;
}

