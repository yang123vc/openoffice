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



#include "precompiled_ucb.hxx"

#include "ucpext_provider.hxx"
#include "ucpext_content.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <ucbhelper/contentidentifier.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <comphelper/componentcontext.hxx>
#include <rtl/ustrbuf.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

	/** === begin UNO using === **/
	using ::com::sun::star::uno::Reference;
	using ::com::sun::star::uno::XInterface;
	using ::com::sun::star::uno::UNO_QUERY;
	using ::com::sun::star::uno::UNO_QUERY_THROW;
	using ::com::sun::star::uno::UNO_SET_THROW;
	using ::com::sun::star::uno::Exception;
	using ::com::sun::star::uno::RuntimeException;
	using ::com::sun::star::uno::Any;
	using ::com::sun::star::uno::makeAny;
	using ::com::sun::star::uno::Sequence;
	using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::ucb::IllegalIdentifierException;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::uno::XComponentContext;
	/** === end UNO using === **/

    //==================================================================================================================
    //= ContentProvider
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ContentProvider::ContentProvider( const Reference< XMultiServiceFactory >& i_rServiceManager )
        :ContentProvider_Base( i_rServiceManager )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ContentProvider::~ContentProvider()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ContentProvider::getImplementationName_static() throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.ucp.ext.ContentProvider" ) );
    }
    
    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ContentProvider::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_static();
    }
    
    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ContentProvider::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(2);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.ContentProvider" ) );
        aServiceNames[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.ExtensionContentProvider" ) );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ContentProvider::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > ContentProvider::Create( const Reference< XComponentContext >& i_rContext )
    {
        const ::comphelper::ComponentContext aContext( i_rContext );
        return *( new ContentProvider( aContext.getLegacyServiceFactory() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString ContentProvider::getRootURL()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.extension://" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString ContentProvider::getArtificialNodeContentType()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.extension-content" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        void lcl_ensureAndTransfer( ::rtl::OUString& io_rIdentifierFragment, ::rtl::OUStringBuffer& o_rNormalization, const sal_Unicode i_nLeadingChar )
        {
            if ( ( io_rIdentifierFragment.getLength() == 0 ) || ( io_rIdentifierFragment[0] != i_nLeadingChar ) )
                throw IllegalIdentifierException();
            io_rIdentifierFragment = io_rIdentifierFragment.copy( 1 );
            o_rNormalization.append( i_nLeadingChar );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XContent > SAL_CALL ContentProvider::queryContent( const Reference< XContentIdentifier  >& i_rIdentifier )
        throw( IllegalIdentifierException, RuntimeException )
    {
        // Check URL scheme...
        const ::rtl::OUString sScheme( rtl::OUString::createFromAscii( "vnd.sun.star.extension" ) );
        if ( !i_rIdentifier->getContentProviderScheme().equalsIgnoreAsciiCase( sScheme ) )
            throw IllegalIdentifierException();

        // normalize the identifier
        const ::rtl::OUString sIdentifier( i_rIdentifier->getContentIdentifier() );

        // the scheme needs to be lower-case
        ::rtl::OUStringBuffer aComposer;
        aComposer.append( sIdentifier.copy( 0, sScheme.getLength() ).toAsciiLowerCase() );

        // one : is required after the scheme
        ::rtl::OUString sRemaining( sIdentifier.copy( sScheme.getLength() ) );
        lcl_ensureAndTransfer( sRemaining, aComposer, ':' );

        // and at least one /
        lcl_ensureAndTransfer( sRemaining, aComposer, '/' );

        // the normalized form requires one additional /, but we also accept identifiers which don't have it
        if ( sRemaining.getLength() == 0 )
        {
            // the root content is a special case, it requires ///
            aComposer.appendAscii( "//" );
        }
        else
        {
            if ( sRemaining[0] != '/' )
            {
                aComposer.append( sal_Unicode( '/' ) );
                aComposer.append( sRemaining );
            }
            else
            {
                lcl_ensureAndTransfer( sRemaining, aComposer, '/' );
                // by now, we moved "vnd.sun.star.extension://" from the URL to aComposer
                if ( sRemaining.getLength() == 0 )
                {
                    // again, it's the root content, but one / is missing
                    aComposer.append( sal_Unicode( '/' ) );
                }
                else
                {
                    aComposer.append( sRemaining );
                }
            }
        }
        const Reference< XContentIdentifier > xNormalizedIdentifier( new ::ucbhelper::ContentIdentifier( m_xSMgr, aComposer.makeStringAndClear() ) );

        ::osl::MutexGuard aGuard( m_aMutex );

        // check if a content with given id already exists...
        Reference< XContent > xContent( queryExistingContent( xNormalizedIdentifier ).get() );
        if ( xContent.is() )
            return xContent;

        // create a new content
        xContent = new Content( m_xSMgr, this, xNormalizedIdentifier );
        if ( !xContent->getIdentifier().is() )
            throw IllegalIdentifierException();

        registerNewContent( xContent );
        return xContent;
    }

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................
