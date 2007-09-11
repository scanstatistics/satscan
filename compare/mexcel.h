#ifndef MINI_EXCEL_H_ALREADY_INCLUDED
#define MINI_EXCEL_H_ALREADY_INCLUDED

#include <stdio.h>

#include <vector>
using namespace std; /* so I can write "vector" instead of "std::vector" */

/* The version of excel we currently support           */
#define EXCEL_VERSION 	2
/* when we write an excel document, we write this type */
#define TYPE_WORKSHEET 	0x10

/* many defies for BIFF Opcodes for different types 
   they also seem to be in the order they appear in an excel document :-)
 */
#define OPCODE_BOF                           0x09
#define OPCODE_FILEPASS                      0x2F
#define OPCODE_INDEX                         0x0B
#define OPCODE_CALCCOUNT                     0x0C
#define OPCODE_CALCMODE                      0x0D
#define OPCODE_PRECISION                     0x0E
#define OPCODE_REFMODE                       0x0F
#define OPCODE_DELTA                         0x10
#define OPCODE_ITERATION                     0x11
#define OPCODE_1904                          0x22
#define OPCODE_BACKUP                        0x40
#define OPCODE_PRINT_ROW_HEADERS             0x2A
#define OPCODE_PRINT_GRIDLINES               0x2B
#define OPCODE_HORIZONTAL_PAGE_BREAKS        0x1B
#define OPCODE_VERTICAL_PAGE_BREAKS          0x1A
#define OPCODE_DEFAULT_ROW_HEIGHT            0x25
#define OPCODE_FONT                          0x31
#define OPCODE_FONT2                         0x32
#define OPCODE_HEADER                        0x14
#define OPCODE_FOOTER                        0x15
#define OPCODE_LEFT_MARGIN                   0x26
#define OPCODE_RIGHT_MARGIN                  0x27
#define OPCODE_TOP_MARGIN                    0x28
#define OPCODE_BOTTOM_MARGIN                 0x29
#define OPCODE_COLWIDTH                      0x24
#define OPCODE_EXTERNCOUNT                   0x16
#define OPCODE_EXTERNSHEET                   0x17
#define OPCODE_EXTERNNAME                    0x23
#define OPCODE_FORMATCOUNT                   0x1F
#define OPCODE_FORMAT                        0x1E
#define OPCODE_NAME                          0x18
#define OPCODE_DIMENSIONS                    0x00
#define OPCODE_COLUMN_DEFAULT                0x20
#define OPCODE_ROW                           0x08
#define OPCODE_BLANK                         0x01
#define OPCODE_INTEGER                       0x02
#define OPCODE_NUMBER                        0x03
#define OPCODE_LABEL                         0x04
#define OPCODE_BOOLERR                       0x05
#define OPCODE_FORMULA                       0x06
#define OPCODE_ARRAY                         0x21
#define OPCODE_CONTINUE                      0x3C
#define OPCODE_STRING                        0x07
#define OPCODE_TABLE                         0x36
#define OPCODE_TABLE2                        0x37
#define OPCODE_PROTECT                       0x12
#define OPCODE_WINDOW_PROTECT                0x19
#define OPCODE_PASSWORD                      0x13
#define OPCODE_NOTE                          0x1C
#define OPCODE_WINDOW1                       0x3D
#define OPCODE_WINDOW2                       0x3E
#define OPCODE_PANE                          0x41
#define OPCODE_SELECTION                     0x1D
#define OPCODE_EOF                           0x0A

/* Alignament options for a cell */
#define	ALIGN_GENERAL  						0
#define	ALIGN_LEFT    						1
#define	ALIGN_CENTER  						2
#define	ALIGN_RIGHT   						3
#define	ALIGN_FILL    						4
#define	ALIGN_MULTIPLAN_DEFAULT  	7	
				
/* the border values for a cell */
#define BORDER_LEFT    0x08
#define BORDER_RIGHT   0x10
#define BORDER_TOP     0x20
#define BORDER_BOTTOM  0x40


/* We put everything into the miniexcel namespace */
namespace miniexcel
{
	/********************************************************
	 * First, define a writer for our excel files           *
	 ********************************************************/
	class LittleEndianWriter{
		private:
			FILE 	*m_pFile; /* where to write */
		public:
		
		  /* a little endian file writer out of a file              */
			LittleEndianWriter(FILE *f);
		
			 /* Destructor will close the file. do not close directly */
		  ~LittleEndianWriter();
			
		  /* Write 1 byte in the output                             */
			void Write1(char v);
		
		  /* Write 2 bytes in the output (little endian order)      */
			void Write2(int v);
		
		  /* Write 4 bytes in the output (little endian order)      */
			void Write4(long v);
		
		  /* Write a 4 byte float in the output                     */
			void WriteFloatIEEE(float v);
		
		  /* Write a 8 byte double in the output                    */
			void WriteDoubleIEEE(double v);			
	};
	
	
  /********************************************************
	 * Everything in Excel is a BIFF record.. so define one *
	 ********************************************************/
	class BIFFRecord{
		protected:    /* Not usefull without subclassing ...        */
			BIFFRecord () {}  				
			virtual ~BIFFRecord () {} 
				
			/* Write a BIFF header for the opcode nRecno of length nRecLen */
			void Write (LittleEndianWriter *pWriter, int nRecNo, int nRecLen);
		public:
			/* We should be able to write every type of BIFF records */
			virtual void Write (LittleEndianWriter *pWriter) = 0; 	
	};	
	
	/* Since every cell has the same attributes, no matter
	   of the type, we gather the attributes here
	 */
	class excelValueAttributes{
		private:
			int							m_nRow;        /* row of the value    */
			int 						m_nColumn;     /* column of the value */		
		    /* all cell attributes held in 3 bytes */
			char						m_nAttr1;  
			char   				     	m_nAttr2;
			char   					    m_nAttr3;
		public:					
		
			excelValueAttributes ();
			excelValueAttributes (int nRow,int nColumn);
			~excelValueAttributes (){}
				
			/* like operator=, but a method */
			void CopyAttributes(excelValueAttributes v){
				m_nRow = v.m_nRow;
				m_nColumn = v.m_nColumn;
				m_nAttr1 = v.m_nAttr1;
				m_nAttr2 = v.m_nAttr2;
				m_nAttr3 = v.m_nAttr3;
			}
			
			/* write them to this endian writer */
			void Write (LittleEndianWriter *pWriter);
				
			/* get/set the position of the current item */
			int  getRow();
			void setRow(int v); 
				
			int  getColumn();
			void setColumn(int v);
			
			/* Tons and tons of attributes that can be set or not... */			
			void setHidden (bool v);
			bool getHidden ();
				
			void setLocked (bool v);
			bool getLocked ();
				
			void setShaded (bool v);
			bool getShaded ();
				
			void setBorder (int type);
			int  getBorder ();
				
			void setAlignament (int type);
			int  getAlignament ();
															
			void setFontNum (int v);
			int  getFontNum ();
			
			void setFormatNum (int v);
			int  getFormatNum ();
				
	};
	
	/* The BOF record. A must have. */
	class excelBOF : public BIFFRecord{
		private:
			int   m_nVersion; /* Version. in our case EXCEL_VERSION    */
			int		m_nType;    /* The type, in our case TYPE_WORKSHEET  */
		public:
			excelBOF (int nVer,int nType):m_nVersion(nVer),m_nType(nType){}
			~excelBOF(){}
			void Write (LittleEndianWriter *pWriter);
		
	};
	
	
	/* A number - 8 byte IEEE double
	 */
	class excelNUMBER : public BIFFRecord,public excelValueAttributes{
		private:
			double m_nValue;
		public:
		
			excelNUMBER (){
				m_nValue = 0;
			}
			
			excelNUMBER (double val){
				m_nValue = val;
			}
		
			void setValue (double v){
				m_nValue = v;
			}
			
			double getValue (){
				return m_nValue;
			}
		
			void Write (LittleEndianWriter *pWriter);
	};	
	
	/* A label: up to 256 character string
	 */
	class excelLABEL : public BIFFRecord,public excelValueAttributes{
		private:
			char		m_pchValue[257];
		public:			 
		
			excelLABEL (){
				m_pchValue[0] = '\0';
			}
			
			excelLABEL (const char *v){
				strncpy (m_pchValue, v, sizeof (m_pchValue));
				m_pchValue[sizeof(m_pchValue) - 1] = '\0';
			}
		
			void setValue (const char *v){
				strncpy (m_pchValue, v, sizeof (m_pchValue));
				m_pchValue[sizeof(m_pchValue) - 1] = '\0';
			}
			
			const char *getValue (){
				return m_pchValue;
			}
		
			void Write (LittleEndianWriter *pWriter);
	};
	
	/* The EOF record. A must have. */
	class excelEOF : public BIFFRecord{
		public:
			excelEOF() {}
			~excelEOF() {}
			void Write (LittleEndianWriter *pWriter);		
	};
	
	/* A generic cell. can contain a number, or a string.
		used so we can easely manipulate cells in excel
	 */
	class ExcelCell:public excelValueAttributes{
		private:
			char 		*m_pchValue;
			double	 m_nValue;
			int			 m_nType;
		public:
			ExcelCell ();
			ExcelCell (const ExcelCell &v);
			~ExcelCell ();
		
			enum{
				TYPE_NONE		= 0,
				TYPE_NUMBER = 1,
				TYPE_STRING = 2
			};
			
			/* if someone is interested in what i contain */
			int getType(){
				return m_nType;
			}
			
			/* make it point to nothing */
			void clear();
			
			/* the copy operator */
			ExcelCell& operator=(const ExcelCell &v);
		
			/* Excell cell can contain a double value */
			ExcelCell& operator=(double v);
			operator double();
		
			/* Excell cell can also contain a string */	
			ExcelCell& operator=(const char *v);
			operator const char *();
			
			void Write (LittleEndianWriter *pWriter);		
			
	};
	

  /********************************************************
	 *  Our class. We will use this class to store an Excel *
	 * Worksheet and write it to a file                     *
	 ********************************************************/
  class CMiniExcel{
		private:
			vector<vector<ExcelCell> > m_vvTableValues;
		public:
			CMiniExcel ();
			~CMiniExcel ();
		
		  /* Access the columns in the excel document */
			ExcelCell &operator() (unsigned row,unsigned column);
		
		 	/* Write into a file. dest must be opened in "wb" mode.
				 the file will be closed by this function
		   */
			void Write (FILE *dest);
  };

};

#endif  /* #ifndef MINI_EXCEL_H_ALREADY_INCLUDED */
