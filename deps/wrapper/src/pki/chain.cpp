#include "../stdafx.h"

#include "wrapper/pki/chain.h"

Handle<CertificateCollection> Chain::buildChain(Handle<Certificate> cert, Handle<CertificateCollection> certs){
	LOGGER_FN();

	try{
		Handle<Certificate> issuer = new Certificate();

		Handle<CertificateCollection> chain = new CertificateCollection();
		chain->push(cert);

		if (cert->isSelfSigned()) {
			return chain;
		}

		Handle<Certificate> xtemp = cert;

		do{
			if ((issuer = getIssued(certs, xtemp)).isEmpty()){
				THROW_EXCEPTION(0, Chain, NULL, "Undefined issuer certificate");
			}

			if (xtemp->compare(issuer) == 0){
				issuer.empty();
			}
			else{
				chain->push(issuer);
				xtemp = issuer;
			}

		} while (!issuer.isEmpty());

		return chain;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Chain, e, "Error build chain (certificate collection)");
	}
}

bool Chain::verifyChain(Handle<CertificateCollection> chain, Handle<CrlCollection> crls){
	LOGGER_FN();

	try{
		bool res = true;

		LOGGER_OPENSSL(X509_STORE_CTX_new);
		X509_STORE_CTX *ctx = X509_STORE_CTX_new();
		if (!ctx) {
			THROW_OPENSSL_EXCEPTION(0, Revocation, NULL, "Error create new store ctx");
		}

		LOGGER_OPENSSL(X509_STORE_new);
		X509_STORE *st = X509_STORE_new();
		if (!st) {
			THROW_OPENSSL_EXCEPTION(0, Revocation, NULL, "Error create new store");
		}

		for (int i = 0, c = chain->length(); i < c; i++){
			LOGGER_OPENSSL(X509_STORE_add_cert);
			X509_STORE_add_cert(st, chain->items(i)->internal());
		}

		LOGGER_OPENSSL(X509_STORE_CTX_init);
		X509_STORE_CTX_init(ctx, st, chain->items(0)->internal(), chain->internal());

		if (crls->length()) {
			LOGGER_OPENSSL(X509_STORE_CTX_set0_crls);
			X509_STORE_CTX_set0_crls(ctx, crls->internal());

			LOGGER_OPENSSL(X509_STORE_CTX_set_flags);
			X509_STORE_CTX_set_flags(ctx, X509_V_FLAG_CRL_CHECK);
			LOGGER_OPENSSL(X509_STORE_CTX_set_flags);
			X509_STORE_CTX_set_flags(ctx, X509_V_FLAG_CRL_CHECK_ALL);
		}

		LOGGER_OPENSSL(X509_STORE_CTX_set_flags);
		X509_STORE_CTX_set_flags(ctx, X509_V_FLAG_CHECK_SS_SIGNATURE);

		LOGGER_OPENSSL(X509_verify_cert);
		if (X509_verify_cert(ctx) <= 0){
			res = false;
		}

		LOGGER_OPENSSL(X509_STORE_CTX_free);
		X509_STORE_CTX_free(ctx);
		ctx = NULL;

		LOGGER_OPENSSL(X509_STORE_free);
		X509_STORE_free(st);
		st = NULL;

		return res;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Chain, e, "Error verify chain (provider store)");
	}	
}

Handle<Certificate> Chain::getIssued(Handle<CertificateCollection> certs, Handle<Certificate> cert){
	LOGGER_FN();

	try{
		for (int i = 0, c = certs->length(); i < c; i++){
			if (checkIssued(certs->items(i), cert)){
				return certs->items(i);
			}
		}

		return NULL;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Chain, e, "Error get issued");
	}
}

bool Chain::checkIssued(Handle<Certificate> issuer, Handle<Certificate> cert){
	LOGGER_FN();

	try{
		int ret;

		if (issuer->isEmpty()){
			THROW_EXCEPTION(0, Chain, NULL, "Empty issuer cert");
		}

		if (cert->isEmpty()){
			THROW_EXCEPTION(0, Chain, NULL, "Empty sub cert");
		}

		LOGGER_OPENSSL(X509_check_issued);
		ret = X509_check_issued(issuer->internal(), cert->internal());
		if (ret == X509_V_OK){
			return 1;
		}
		else{
			return 0;
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Chain, e, "checkIssued");
	}	
}

