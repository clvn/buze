#include "it_enum_list.h"
//#include <stdio.h>
#include <stdlib.h>

int it_enum_list::StringToInt(const SeSdkString2 &string, int p_base)
{
//	assert( string.GetLength() < 9 || string.Find(_T(",")) != 0 ); // else too many digits overflows sizeof int
	wchar_t * temp;
	return wcstol( string.c_str(), &temp, p_base );
}


it_enum_list::it_enum_list(const SeSdkString2 &p_enum_list) :
m_enum_list(p_enum_list)
{
	// two formats are allowed, a list of values or a range
//	if( m_enum_list.Left(5) == SeSdkString2(_T("range")) )
	if( m_enum_list.substr(0,5) == SeSdkString2( L"range" ) ) // how about upper/mixed case?
	{
		m_range_mode = true;
		SeSdkString2 list = m_enum_list;

//		list = list.Right( list.GetLength() - 5);
		list = list.substr( 6, list.size() - 1);

		int p = list.find(L",");
		assert( p != -1 );	// no range specified

		range_lo = StringToInt( list );
//		list = list.Right( list.GetLength() - p -1);
		list = list.substr( p + 1, list.size() - 1);
		range_hi = StringToInt( list );
	}
	else
	{
		m_range_mode = false;
	}
}

void it_enum_list::First()
{
	// two formats are allowed, a list of values or a range
	if( m_range_mode )
	{
		m_current.value = range_lo - 1;
//		m_current.text.Format(_T("%d"),range_lo);
//		m_current.index = 0;
	}
	else
	{
/*
#if defined( _DEBUG )
		m_temp_list = m_enum_list;
#endif
*/
		range_lo = 0;
		m_current.value = -1;
	}
	m_current.index = -1; // anticipate initial next()
	Next();
}

void it_enum_list::Next()
{
	m_current.index++;
	m_current.value++;

	if(m_range_mode)
	{
//		m_current.text.Format(_T("%d"), m_current.value);
		wchar_t temp[16];
		swprintf( temp, L"%d", m_current.value);
		m_current.text = temp;
		if(m_current.value > range_hi)
		{
			m_current.index = -1;
			return;
		}
	}
	else
	{
//		{
		// find next comma
//		int p = m_enum_list.Find(',', range_lo);
		int p = m_enum_list.find(L",", range_lo);
		if( p == -1 ) // none left
		{
//			p = m_enum_list.GetLength();
			p = m_enum_list.size();

			if( range_lo >= p ) // then we are done
			{
				m_current.index = -1;
				return;
			}
		}

		int sub_string_length = p - range_lo;
//		m_current.text = m_enum_list.Mid( range_lo, sub_string_length);
		m_current.text = m_enum_list.substr( range_lo, sub_string_length);

//		int p_eq = m_current.text.Find('=');
		int p_eq = m_current.text.find(L"=");
		if( p_eq > 0 )
		{
//			SeSdkString2 id_str = m_current.text.Right(sub_string_length - p_eq - 1 );
			SeSdkString2 id_str = m_current.text.substr(p_eq + 1, m_current.text.size() - 1 );
//			m_current.text = m_current.text.Left( p_eq );
			m_current.text = m_current.text.substr( 0, p_eq );
			m_current.value = StringToInt(id_str);
		}

		range_lo = p + 1;

		// Trim spaces from start and end of text
//		m_current.text.Trim();
		int st = m_current.text.find_first_not_of(L" ");
		int en = m_current.text.size();
		while(en > 0 && m_current.text[en - 1] == L' ' )
		{
			--en;
		}

		if( st > 0 || en < m_current.text.size() )
		{
			m_current.text = m_current.text.substr(st,en-st);
		}
	}
}

int it_enum_list::size(void)
{
	if(m_range_mode)
	{
		return 1 + abs(range_hi - range_lo);
	}
	else
	{
		// count number of commas
		int sz = 1;
//		for( int i = m_enum_list.GetLength() - 1 ; i > 0 ; i-- )
		for( int i = m_enum_list.size() - 1 ; i > 0 ; i-- )
		{
			if( m_enum_list[i] == ',' )
				sz++;
		}
		return sz;
	}
}

bool it_enum_list::FindValue( int p_value )
{
	// could be specialied for ranges

	for( First(); !IsDone(); Next() )
	{
		if( CurrentItem()->value == p_value )
			return true;
	}
	return false;
}

bool it_enum_list::FindIndex( int p_index )
{
	for( First(); !IsDone(); Next() )
	{
		if( CurrentItem()->index == p_index )
			return true;
	}
	return false;
}

bool it_enum_list::IsValidValue( const SeSdkString2 &p_enum_list, int p_value )
{
	it_enum_list itr(p_enum_list);
	return itr.FindValue(p_value);
}

// ensure a value is one of the valid choices, if not return first item, if no items avail return 0
int it_enum_list::ForceValidValue( const SeSdkString2 &p_enum_list, int p_value )
{
	it_enum_list itr(p_enum_list);
	if( itr.FindValue(p_value) )
		return p_value;

	itr.First();
	if( !itr.IsDone() )
		return itr.CurrentItem()->value;

	return 0;
}
/*
float it_enum_list::ValueToNormalised( int p_value )
{
	float number_of_values = (float) itr.size();
	int index = (int) floorf( p_value * number_of_values + 0.5f); // floor important for -ve numbers
	itr.FindIndex( index );
	assert( !itr.IsDone() );
	setValue( IntToString( itr.CurrentItem()->value ) );
}
*/
