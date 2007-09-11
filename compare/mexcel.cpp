#include "mexcel.h"

#include <string.h>

namespace miniexcel
{

  /* Attach the writer to the specified file */
  LittleEndianWriter::LittleEndianWriter (FILE * f)
  {
    m_pFile = f;
  }

  /* Destructor closes the file itself */
  LittleEndianWriter::~LittleEndianWriter ()
  {
    fclose (m_pFile);
  }

  /* Write 1 byte in the output */
  void LittleEndianWriter::Write1 (char v)
  {
    fwrite (&v, 1, 1, m_pFile);
  }

  /* Write 2 bytes in the output (little endian order) */
  void LittleEndianWriter::Write2 (int v)
  {
    Write1 ((v) & 0xff);
    Write1 ((v >> 8) & 0xff);
  }

  /* Write 4 bytes in the output (little endian order) */
  void LittleEndianWriter::Write4 (long v)
  {
    Write2 ((v) & 0xffff);
    Write2 ((v >> 16) & 0xffff);
  }

  /* Write a 4 byte float in the output */
  void LittleEndianWriter::WriteFloatIEEE (float v)
  {
    fwrite (&v, 1, sizeof (v), m_pFile);
  }

  /* Write a 8 byte double in the output */
  void LittleEndianWriter::WriteDoubleIEEE (double v)
  {
    fwrite (&v, 1, sizeof (v), m_pFile);
  }

  /* Write a BIFF header for the opcode nRecno of length nRecLen */
  void BIFFRecord::Write (LittleEndianWriter * pWriter, int nRecNo, int nRecLen)
  {
    pWriter->Write2 (nRecNo);
    pWriter->Write2 (nRecLen);
  }

  /* default constructor for our excel attributes */
  excelValueAttributes::excelValueAttributes () {
    m_nRow = m_nColumn = 0;
    m_nAttr1 = m_nAttr2 = m_nAttr3 = 0;
  }

  /* set the row/column of these values */
  excelValueAttributes::excelValueAttributes (int nRow, int nColumn)
  {
    m_nRow = nRow;
    m_nColumn = nColumn;
    m_nAttr1 = m_nAttr2 = m_nAttr3 = 0;
  }

  /* write them to this endian writer */
  void excelValueAttributes::Write (LittleEndianWriter * pWriter)
  {
    pWriter->Write2 (m_nRow);
    pWriter->Write2 (m_nColumn);
    pWriter->Write1 (m_nAttr1);
    pWriter->Write1 (m_nAttr2);
    pWriter->Write1 (m_nAttr3);
  }

  int excelValueAttributes::getRow ()
  {
    return m_nRow;
  }

  void excelValueAttributes::setRow (int v)
  {
    m_nRow = v;
  }


  int excelValueAttributes::getColumn ()
  {
    return m_nColumn;
  }

  void excelValueAttributes::setColumn (int v)
  {
    m_nColumn = v;
  }

  void excelValueAttributes::setHidden (bool v)
  {
    if (v) {
      m_nAttr1 |= 0x80;
    } else {
      m_nAttr1 &= ~0x80;
    }
  }

  bool excelValueAttributes::getHidden ()
  {
    return (m_nAttr1 & 0x80) != 0;
  }

  void excelValueAttributes::setLocked (bool v)
  {
    if (v) {
      m_nAttr1 |= 0x40;
    } else {
      m_nAttr1 &= ~0x40;
    }
  }

  bool excelValueAttributes::getLocked ()
  {
    return (m_nAttr1 & 0x40) != 0;
  }

  void excelValueAttributes::setShaded (bool v)
  {
    if (v) {
      m_nAttr3 |= 0x80;
    } else {
      m_nAttr3 &= ~0x80;
    }
  }

  bool excelValueAttributes::getShaded ()
  {
    return (m_nAttr3 & 0x80) != 0;
  }

  void excelValueAttributes::setBorder (int type)
  {
    m_nAttr3 &= ~0x78;          /* clear existing border */
    m_nAttr3 |= (type & 0x78);  /* set the new border    */
  }

  int excelValueAttributes::getBorder ()
  {
    return m_nAttr3 & 0x78;
  }

  void excelValueAttributes::setAlignament (int type)
  {
    m_nAttr3 &= ~0x07;          /* clear previous value */
    m_nAttr3 |= type & 0x07;
  }

  int excelValueAttributes::getAlignament ()
  {
    return m_nAttr3 & 0x07;
  }

  void excelValueAttributes::setFontNum (int v)
  {
    m_nAttr2 &= ~0xE0;          /* clear previous value */
    m_nAttr2 |= (v & 0x03) << 5;        /* set the new value value */
  }

  int excelValueAttributes::getFontNum ()
  {
    return (m_nAttr2 >> 5) & 0x03;
  }

  void excelValueAttributes::setFormatNum (int v)
  {
    m_nAttr2 &= ~0x3F;          /* clear previous value */
    m_nAttr2 |= v & 0x3F;       /* set the new value value */
  }

  int excelValueAttributes::getFormatNum ()
  {
    return m_nAttr2 & 0x3F;
  }

  /* write a BOF record */
  void excelBOF::Write (LittleEndianWriter * pWriter)
  {
    BIFFRecord::Write (pWriter, OPCODE_BOF, 4);
    pWriter->Write2 (m_nVersion);
    pWriter->Write2 (m_nType);
  }

  /* write a number */
  void excelNUMBER::Write (LittleEndianWriter * pWriter)
  {
    BIFFRecord::Write (pWriter, OPCODE_NUMBER, 15);
    excelValueAttributes::Write (pWriter);
    pWriter->WriteDoubleIEEE (m_nValue);
  }

  /* write a label */
  void excelLABEL::Write (LittleEndianWriter * pWriter)
  {
    BIFFRecord::Write (pWriter, OPCODE_LABEL, 8 + strlen (m_pchValue));
    excelValueAttributes::Write (pWriter);
    pWriter->Write1 (strlen (m_pchValue));
    for (unsigned i = 0; i < strlen (m_pchValue); i++) {
      pWriter->Write1 (m_pchValue[i]);
    }
  }

  void excelEOF::Write (LittleEndianWriter * pWriter)
  {
    BIFFRecord::Write (pWriter, OPCODE_EOF, 0);
  }

  ExcelCell::ExcelCell () {
    m_pchValue = NULL;
    m_nValue = 0;
    m_nType = TYPE_NONE;
  }
	
  ExcelCell::ExcelCell(const ExcelCell &v){		
	m_nType = v.m_nType;
	if (m_nType == TYPE_STRING){
	  m_pchValue = strdup (v.m_pchValue);
	}else{
      m_pchValue = NULL;
	}
	m_nValue = v.m_nValue;
	CopyAttributes(v);
  }

  ExcelCell::~ExcelCell () {
    if (m_pchValue != NULL) {
      free (m_pchValue);
    }
  }

  /* Excell cell can contain a double value */
  ExcelCell & ExcelCell::operator = (double v) {
    m_nType = TYPE_NUMBER;
    m_nValue = v;
    return *this;
  }

  ExcelCell::operator double ()
  {
    return m_nValue;
  }
	
  /* Excell cell can also contain a string */
  ExcelCell & ExcelCell::operator = (const char *v) {
		m_nType = TYPE_STRING;
    if (m_pchValue != NULL) {
      free (m_pchValue);
    }
    m_pchValue = strdup (v);    /* FIXME: check for NULL */
    return *this;
  }

  ExcelCell::operator const char *()
  {
    return m_pchValue;
  }

  void ExcelCell::clear ()
  {
    if (m_pchValue != NULL) {
      free (m_pchValue);
      m_pchValue = NULL;
    }
    m_nType = TYPE_NONE;
  }
	
  ExcelCell& ExcelCell::operator=(const ExcelCell &v){
	if (m_pchValue != NULL) {
      free (m_pchValue);     
      m_pchValue = NULL;
    }
    m_nType = v.m_nType;
    if (m_nType == TYPE_STRING){
      m_pchValue = strdup (v.m_pchValue);
    }
    m_nValue = v.m_nValue;
	CopyAttributes(v);
    return *this;
  }

  void ExcelCell::Write (LittleEndianWriter * pWriter)
  {
    if (m_nType == TYPE_NONE) {
      return;                   /* Do nothing if we have no actual value */
    }
    if (m_nType == TYPE_NUMBER) {
      excelNUMBER n (m_nValue);

      n.CopyAttributes (*this);
      n.Write (pWriter);
    } else {
      //ASSERT: m_nType == TYPE_STRING
      excelLABEL n (m_pchValue);

      n.CopyAttributes (*this);
      n.Write (pWriter);
    }
  }

  ExcelCell & CMiniExcel::operator ()(unsigned row, unsigned column) {
    while (m_vvTableValues.size () <= row) {
      vector < ExcelCell > v;
      m_vvTableValues.push_back (v);    /* add any extra rows */
    }
    while (m_vvTableValues[row].size () <= column) {
      ExcelCell v;

      m_vvTableValues[row].push_back (v);       /* add any extra columns */
    }
    return m_vvTableValues[row][column];
  }
	
	CMiniExcel::CMiniExcel(){
	}
	
	CMiniExcel::~CMiniExcel(){
	}

  void CMiniExcel::Write (FILE * dest)
  {
    LittleEndianWriter writer (dest);
    excelBOF biffBOF (EXCEL_VERSION, TYPE_WORKSHEET);   /* begin and end of file */
    excelEOF biffEOF;
    unsigned row, column;

    biffBOF.Write (&writer);

    /* first pass, gather all the numbers */
    for (row = 0; row < m_vvTableValues.size (); row++) {
      for (column = 0; column < m_vvTableValues[row].size (); column++) {
        if (m_vvTableValues[row][column].getType () == ExcelCell::TYPE_NUMBER) {
          m_vvTableValues[row][column].setRow (row);
          m_vvTableValues[row][column].setColumn (column);
          m_vvTableValues[row][column].Write (&writer);
        }
      }
    }

    /* second pass for the strings */
    for (row = 0; row < m_vvTableValues.size (); row++) {
      for (column = 0; column < m_vvTableValues[row].size (); column++) {
        if (m_vvTableValues[row][column].getType () == ExcelCell::TYPE_STRING) {
          m_vvTableValues[row][column].setRow (row);
          m_vvTableValues[row][column].setColumn (column);
          m_vvTableValues[row][column].Write (&writer);
        }
      }
    }

    biffEOF.Write (&writer);

  }

}
