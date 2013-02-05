/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <basic/sbx.hxx>
#include <tools/errcode.hxx>

#define _TLBIGINT_INT64
#include <tools/bigint.hxx>

#include <basic/sbxvar.hxx>
#include "sbxconv.hxx"

static ::rtl::OUString   ImpCurrencyToString( const SbxINT64& );
static SbxINT64 ImpStringToCurrency( const ::rtl::OUString& );

SbxINT64 ImpGetCurrency( const SbxValues* p )
{
	SbxValues aTmp;
	SbxINT64 nRes;
start:
	switch( +p->eType )
	{
		case SbxNULL:
			SbxBase::SetError( SbxERR_CONVERSION );
		case SbxEMPTY:
			nRes.SetNull(); break;
		case SbxCHAR:
			nRes = ImpDoubleToCurrency( (double)p->nChar ); break;
		case SbxBYTE:
			nRes = ImpDoubleToCurrency( (double)p->nByte ); break;
		case SbxINTEGER:
		case SbxBOOL:
			nRes = ImpDoubleToCurrency( (double)p->nInteger ); break;
		case SbxERROR:
		case SbxUSHORT:
			nRes = ImpDoubleToCurrency( (double)p->nUShort ); break;
		case SbxCURRENCY:
			nRes = p->nLong64; break;
		case SbxLONG:
			nRes = ImpDoubleToCurrency( (double)p->nLong );
			break;
		case SbxULONG:
			nRes = ImpDoubleToCurrency( (double)p->nULong );
			break;
		case SbxSALINT64:
			nRes = ImpDoubleToCurrency( (double)p->nInt64 );
			break;
		case SbxSALUINT64:
			nRes = ImpDoubleToCurrency( ImpSalUInt64ToDouble( p->uInt64 ) );
			break;
		case SbxSINGLE:
			if( p->nSingle > SbxMAXCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMax();
			}
			else if( p->nSingle < SbxMINCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMin();
			}
			else
				nRes = ImpDoubleToCurrency( (double)p->nSingle );
			break;
		case SbxDATE:
		case SbxDOUBLE:
			if( p->nDouble > SbxMAXCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMax();
			}
			else if( p->nDouble < SbxMINCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMin();
			}
			else
				nRes = ImpDoubleToCurrency( p->nDouble );
			break;
		case SbxDECIMAL:
		case SbxBYREF | SbxDECIMAL:
			{
			double d = 0.0;
			if( p->pDecimal )
				p->pDecimal->getDouble( d );
			if( d > SbxMAXCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMax();
			}
			else if( d < SbxMINCURR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes.SetMin();
			}
			else
				nRes = ImpDoubleToCurrency( d );
			break;
			}
		case SbxBYREF | SbxSTRING:
		case SbxSTRING:
		case SbxLPSTR:
			if( !p->pOUString )
				nRes.SetNull();
			else
				nRes = ImpStringToCurrency( *p->pOUString );
			break;
		case SbxOBJECT:
		{
			SbxValue* pVal = dynamic_cast< SbxValue* >( p->pObj);
			if( pVal )
				nRes = pVal->GetCurrency();
			else
			{
				SbxBase::SetError( SbxERR_NO_OBJECT ); nRes.SetNull();
			}
			break;
		}

		case SbxBYREF | SbxCHAR:
			nRes = ImpDoubleToCurrency( (double)*p->pChar ); break;
		case SbxBYREF | SbxBYTE:
			nRes = ImpDoubleToCurrency( (double)*p->pByte ); break;
		case SbxBYREF | SbxINTEGER:
		case SbxBYREF | SbxBOOL:
			nRes = ImpDoubleToCurrency( (double)*p->pInteger ); break;
		case SbxBYREF | SbxERROR:
		case SbxBYREF | SbxUSHORT:
			nRes = ImpDoubleToCurrency( (double)*p->pUShort ); break;
		case SbxBYREF | SbxCURRENCY:
			nRes = *p->pLong64; break;

		// ab hier muss getestet werden
		case SbxBYREF | SbxLONG:
			aTmp.nLong = *p->pLong; goto ref;
		case SbxBYREF | SbxULONG:
			aTmp.nULong = *p->pULong; goto ref;
		case SbxBYREF | SbxSINGLE:
			aTmp.nSingle = *p->pSingle; goto ref;
		case SbxBYREF | SbxDATE:
		case SbxBYREF | SbxDOUBLE:
			aTmp.nDouble = *p->pDouble; goto ref;
		case SbxBYREF | SbxSALINT64:
			aTmp.nInt64 = *p->pnInt64; goto ref;
		case SbxBYREF | SbxSALUINT64:
			aTmp.uInt64 = *p->puInt64; goto ref;
		ref:
			aTmp.eType = SbxDataType( p->eType & 0x0FFF );
			p = &aTmp; goto start;

		default:
			SbxBase::SetError( SbxERR_CONVERSION ); nRes.SetNull();
	}
	return nRes;
}

void ImpPutCurrency( SbxValues* p, const SbxINT64 &r )
{
	double dVal = ImpCurrencyToDouble( r );
	SbxValues aTmp;
start:
	switch( +p->eType )
	{
		// Hier sind Tests notwendig
		case SbxCHAR:
			aTmp.pChar = &p->nChar; goto direct;
		case SbxBYTE:
			aTmp.pByte = &p->nByte; goto direct;
		case SbxINTEGER:
		case SbxBOOL:
			aTmp.pInteger = &p->nInteger; goto direct;
		case SbxLONG:
			aTmp.pLong = &p->nLong; goto direct;
		case SbxULONG:
			aTmp.pULong = &p->nULong; goto direct;
		case SbxERROR:
		case SbxUSHORT:
			aTmp.pUShort = &p->nUShort; goto direct;
		direct:
			aTmp.eType = SbxDataType( p->eType | SbxBYREF );
			p = &aTmp; goto start;

		// ab hier nicht mehr
		case SbxSINGLE:
			p->nSingle = (float)dVal; break;
		case SbxDATE:
		case SbxDOUBLE:
			p->nDouble = dVal; break;
		case SbxSALINT64:
            p->nInt64 = ImpDoubleToSalInt64( dVal ); break;
		case SbxSALUINT64:
			p->uInt64 = ImpDoubleToSalUInt64( dVal ); break;
		case SbxCURRENCY:
			p->nLong64 = r; break;
		case SbxDECIMAL:
		case SbxBYREF | SbxDECIMAL:
			{
			SbxDecimal* pDec = ImpCreateDecimal( p );
			if( !pDec->setDouble( dVal ) )
				SbxBase::SetError( SbxERR_OVERFLOW ); 
			break;
			}
		case SbxBYREF | SbxSTRING:
		case SbxSTRING:
		case SbxLPSTR:
			if( !p->pOUString )
				p->pOUString = new ::rtl::OUString;

			*p->pOUString = ImpCurrencyToString( r );
			break;
		case SbxOBJECT:
		{
			SbxValue* pVal = dynamic_cast< SbxValue* >( p->pObj);
			if( pVal )
				pVal->PutCurrency( r );
			else
				SbxBase::SetError( SbxERR_NO_OBJECT );
			break;
		}
		case SbxBYREF | SbxCHAR:
			if( dVal > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXCHAR;
			}
			else if( dVal < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMINCHAR;
			}
			*p->pChar = (xub_Unicode) dVal; break;
		case SbxBYREF | SbxBYTE:
			if( dVal > SbxMAXBYTE )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXBYTE;
			}
			else if( dVal < 0 )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = 0;
			}
			*p->pByte = (sal_uInt8) dVal; break;
		case SbxBYREF | SbxINTEGER:
		case SbxBYREF | SbxBOOL:
			if( dVal > SbxMAXINT )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXINT;
			}
			else if( dVal < SbxMININT )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMININT;
			}
			*p->pInteger = (sal_Int16) dVal; break;
		case SbxBYREF | SbxERROR:
		case SbxBYREF | SbxUSHORT:
			if( dVal > SbxMAXUINT )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXUINT;
			}
			else if( dVal < 0 )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = 0;
			}
			*p->pUShort = (sal_uInt16) dVal; break;
		case SbxBYREF | SbxLONG:
			if( dVal > SbxMAXLNG )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXLNG;
			}
			else if( dVal < SbxMINLNG )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMINLNG;
			}
			*p->pLong = (sal_Int32) dVal; break;
		case SbxBYREF | SbxULONG:
			if( dVal > SbxMAXULNG )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = SbxMAXULNG;
			}
			else if( dVal < 0 )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); dVal = 0;
			}
			*p->pULong = (sal_uInt32) dVal; break;
		case SbxBYREF | SbxSALINT64:
            *p->pnInt64 = ImpDoubleToSalInt64( dVal ); break;
		case SbxBYREF | SbxSALUINT64:
			*p->puInt64 = ImpDoubleToSalUInt64( dVal ); break;
		case SbxBYREF | SbxSINGLE:
			*p->pSingle = (float) dVal; break;
		case SbxBYREF | SbxDATE:
		case SbxBYREF | SbxDOUBLE:
			*p->pDouble = (double) dVal; break;
		case SbxBYREF | SbxCURRENCY:
			*p->pLong64 = r; break;

		default:
			SbxBase::SetError( SbxERR_CONVERSION );
	}
}

// Hilfs-Funktionen zur Wandlung

static ::rtl::OUString ImpCurrencyToString( const SbxINT64 &r )
{
	BigInt a10000 = 10000;

	//return GetpApp()->GetAppInternational().GetCurr( BigInt( r ), 4 );
	BigInt aInt( r );
	aInt.Abs();
	BigInt aFrac = aInt;
	aInt  /= a10000;
	aFrac %= a10000;
	aFrac += a10000;

	::rtl::OUString aString;
	if( r.nHigh < 0 )
		aString = ::rtl::OUString( (sal_Unicode)'-' );
	aString += aInt.GetString();
	aString += ::rtl::OUString( (sal_Unicode)'.' );
	aString += aFrac.GetString().GetBuffer()+1;
	return aString;
}

static SbxINT64 ImpStringToCurrency( const ::rtl::OUString &r )
{
	int nDec = 4;
	String aStr;
	const sal_Unicode* p = r.getStr();

	if( *p == '-' )
		aStr += *p++;

	while( *p >= '0' && *p <= '9' ) {
		aStr += *p++;
		if( *p == ',' )
			p++;
	}

	if( *p == '.' ) {
		p++;
		while( nDec && *p >= '0' && *p <= '9' ) {
			aStr += *p++;
			nDec--;
		}
	}
	while( nDec ) {
		aStr += '0';
		nDec--;
	}

	BigInt aBig( aStr );
	SbxINT64 nRes;
	aBig.INT64( &nRes );
	return nRes;
}

double ImpINT64ToDouble( const SbxINT64 &r )
{ return (double)r.nHigh*(double)4294967296.0 + (double)r.nLow; }

SbxINT64 ImpDoubleToINT64( double d )
{
	SbxINT64 nRes;
	nRes.Set( d );
	return nRes;
}

double ImpUINT64ToDouble( const SbxUINT64 &r )
{ return (double)r.nHigh*(double)4294967296.0 + (double)r.nLow; }

SbxUINT64 ImpDoubleToUINT64( double d )
{
	SbxUINT64 nRes;
	nRes.Set( d );
	return nRes;
}
