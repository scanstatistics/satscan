//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "stsFrmSuggestedCitation.h"
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmSuggestedCitation *frmSuggestedCitation;
//---------------------------------------------------------------------------
__fastcall TfrmSuggestedCitation::TfrmSuggestedCitation(TComponent* Owner) : TForm(Owner) {

  RichEdit1->Color = Color;      
  ::HideCaret(RichEdit1->Handle);

  unsigned mask = SendMessage(RichEdit1->Handle, EM_GETEVENTMASK, 0, 0);
  SendMessage(RichEdit1->Handle, EM_SETEVENTMASK, 0, mask | ENM_LINK);
  //SendMessage(RichEdit1->Handle, EM_AUTOURLDETECT, true, 0);

  RichEdit1->Lines->Clear();
  RichEdit1->Lines->Add("The SaTScan software may be used freely, with the requirement that proper references are "
                        "provided to the scientific papers describing the statistical methods. For the most common"
                        "analyses, the suggested citations are:");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Bernoulli and Poisson Models: Kulldorff M. A spatial scan statistic. Communications in "
                        "Statistics: Theory and Methods, 26:1481-1496, 1997. [online]");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Space-Time Permutation Model: Kulldorff M, Heffernan R, Hartman J, Assunção RM, "
                        "Mostashari F. A space-time permutation scan statistic for the early detection of disease "
                        "outbreaks. PLoS Medicine, 2:216-224, 2005. [online]");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Ordinal Model: Jung I, Kulldorff M, Klassen A. A spatial scan statistic for ordinal data. "
                        "Manuscript,2005.[online]");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Exponential Model: Huang L, Kulldorff M, Gregorio D. A spatial scan statistic for survival data. "
                        "Manuscript, 2005.[online]");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Software: Kulldorff M. and Information Management Services, Inc. SaTScan(TM) v6.1: Software "
                        "for the spatial and space-time scan  statistics. www.satscan.org, 2006.");
  RichEdit1->Lines->Add("");
  RichEdit1->Lines->Add("Users of SaTScan should in any reference to the software note that:");
  RichEdit1->Lines->Add("\"SaTScan(TM) is a trademark of Martin Kulldorff. The SaTScan(TM) software was developed "
                        "under the joint auspices of (i) Martin Kulldorff, (ii) the National Cancer Institute, and (iii) Farzad "
                        "Mostashari of the New York City Department of Health and Mental Hygiene.\"");

  gvCitationUrls.push_back("http://www.satscan.org/papers/k-cstm1997.pdf");
  gvCitationUrls.push_back("http://medicine.plosjournals.org/archive/1549-1676/2/3/pdf/10.1371_journal.pmed.0020059-L.pdf");
  gvCitationUrls.push_back("http://www.satscan.org/papers/jung-M2005.pdf");
  gvCitationUrls.push_back("http://www.satscan.org/papers/huang-M2005.pdf");
  gvCitationUrls.push_back("http://www.satscan.org");

  ZdString sTextBuffer = RichEdit1->Text.c_str();
  AnsiString   sOnline("online");
  size_t  tUrlIndex=0;
  long iPos=0;
  while ((iPos = sTextBuffer.Find(sOnline.c_str(), iPos)) != -1) {
    FormatRichEditText(iPos, iPos + sOnline.Length());
    gmCharRangeMinToUrlIndex[iPos] = tUrlIndex++;
    iPos += sOnline.Length();
  }
  sOnline = "www.satscan.org";
  iPos=0;
  while ((iPos = sTextBuffer.Find(sOnline.c_str(), iPos)) != -1) {
    FormatRichEditText(iPos, iPos + sOnline.Length());
    gmCharRangeMinToUrlIndex[iPos] = tUrlIndex++;
    iPos += sOnline.Length();
  }
}
//---------------------------------------------------------------------------
void TfrmSuggestedCitation::FormatRichEditText(long lMin, long lMax) {
   CHARRANGE crng;
   crng.cpMin = lMin; crng.cpMax = lMax;
   SendMessage(RichEdit1->Handle, EM_EXSETSEL, 0, LPARAM(&crng));
   Richedit::CHARFORMAT2 cfmt2;
   cfmt2.cbSize = sizeof(Richedit::CHARFORMAT2);
   SendMessage(RichEdit1->Handle, EM_GETCHARFORMAT, SCF_SELECTION, LPARAM(&cfmt2));
   cfmt2.dwEffects |= CFE_LINK;
   SendMessage(RichEdit1->Handle, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&cfmt2));
   crng.cpMin = 0; crng.cpMax = 0;
   SendMessage(RichEdit1->Handle, EM_EXSETSEL, 0, LPARAM(&crng));
}
//---------------------------------------------------------------------------
void __fastcall TfrmSuggestedCitation::WndProc(Messages::TMessage &Message) {

  if (Message.Msg == WM_NOTIFY) {
    switch (((LPNMHDR)Message.LParam)->code) {
      case EN_LINK:
       {
        ENLINK* pEnlink = (ENLINK *)Message.LParam;
        switch (pEnlink->msg) {
          case WM_LBUTTONDOWN :
            {
              std::map<long, size_t>::const_iterator itr=gmCharRangeMinToUrlIndex.find(pEnlink->chrg.cpMin);
              if (itr != gmCharRangeMinToUrlIndex.end())
                ShellExecute(Handle, "open", gvCitationUrls.at(itr->second).c_str(), 0, 0, SW_SHOWDEFAULT);
            }
            break;
          case WM_LBUTTONDBLCLK :
          case WM_LBUTTONUP :
          case WM_MOUSEMOVE :
            {
              std::map<long, size_t>::const_iterator itr=gmCharRangeMinToUrlIndex.find(pEnlink->chrg.cpMin);
              if (itr != gmCharRangeMinToUrlIndex.end())
                 StatusBar1->SimpleText = gvCitationUrls.at(itr->second).c_str();
            }
          case WM_RBUTTONDBLCLK :
          case WM_RBUTTONDOWN :
          case WM_RBUTTONUP :
          case WM_SETCURSOR : break;
        }
      }
      break;
    }
  }
  else if (StatusBar1) StatusBar1->SimpleText = "";

  TForm::WndProc(Message);
}
//---------------------------------------------------------------------------

void __fastcall TfrmSuggestedCitation::ApplicationEvents1Idle(TObject *Sender, bool &Done)
{
  ::HideCaret(RichEdit1->Handle) ;
}
//---------------------------------------------------------------------------

void __fastcall TfrmSuggestedCitation::FormResize(TObject *Sender) {
  RichEdit1->Repaint();
}
//---------------------------------------------------------------------------

