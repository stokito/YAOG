#ifndef SSLCERTIFICATES_H
#define SSLCERTIFICATES_H

#include <QDebug>

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <openssl/ssl.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define KeyRSA 1
#define KeyDSA 2
#define KeyEC 3

#define OPENSSL_BAD_PASSWORD_ERR 104
#define OPENSSL_BAD_DECRYPT_ERR 100
#define MAX_SSL_ERRORS 100  // max number of ssl errors to store
#define MAX_SSL_ERRORS_BUF_SIZE 2000

typedef struct X509_name_st X509_NAME2; // To bypass wincrypt definition.

/**
 * @brief The SSLCertificates class
 */
class SSLCertificates
{
public:
    /**
     * @brief Class constructor. Initialise certificate,csr,key... pointers
     */
    SSLCertificates();
    ~SSLCertificates();

    /**
     * @brief set_key_params : set parameters for all keys
     * @param keyparam : size for type RSA or DSA
     * @param keytype : RSA/ DSA/ EC : set KeyRSA / KeyDSA or KeyEC macro definitions
     * @param ec : name of eliptic curve (list is keyECList) or NULL
     * @return  0 on success, 1 on error
     */
    int set_key_params(unsigned int keyparam, int keytype, char*ec = nullptr);
    int create_key();
    int get_key_PEM(char* Skey,size_t maxlength);
    int get_key_HUM(char* Skey,size_t maxlength);
    /**
     * @brief Reads key in PEM format and put it in EVP_PKEY structure
     * @param Skey : key in PEM format
     * @param password : password to decrypt key. Can be NULL
     * @return 0 on success, 1:Error reading key (error codes set), 2: invalid /missing password 3: unknown key type
     */
    int set_key_PEM(const char* Skey, char *password);
    int get_key_PEM_enc(char *Skey, size_t maxlength, char* password);
    /**
     * @brief get_key_type (key must be loaded)
     * @return key type KeyRSA/DSA/EC or 0 on error
     */
    int get_key_type();
    /**
     * @brief check_key : check if it's a valid key (key must be loaded)
     * @return 0 if loaded key is valid, 1 on error
     */
    int check_key();
    /**
     * @brief check_key_cert_match : check certificate/key match. Key & cert already loaded
     * @return 0 on success 1 on no match, 2 on loading key/cert error
     */
    int check_key_cert_match();
    /**
     * @brief check_key_csr_match : check CSR/key match. Key & CSR already loaded
     * @return 0 on success 1 on no match, 2 on loading key/cert error
     */
    int check_key_csr_match();

    int add_cert_object_byname(const char* label, const char *content);
    int set_object(const char* oCN, const char* oC, const char* oS,
                   const char* oL, const char* oO, const char* oOU,
                   const char *omail);
    /**
     * @brief create_cert
     * @return 0: success, 1: SSL error, 2: certificate validity error
     * check ssl errors
     */
    int create_cert();
    /**
     * @brief get_cert_PEM : put current cert as Pem in skey
     * @param Skey
     * @param maxlength : of skey
     * @param locX509 : optionnal X509* to get pem from
     * @return 0: sucess, 1 : error copying, 2: maxlength too small, 3: error getting cert (note return of get_pkcs12_certs_pem)
     * check ssl errors
     */
    int get_cert_PEM(char* Skey,size_t maxlength,X509* locX509=nullptr);
    /**
     * @brief get_cert_HUM : put cert as text human readeable
     * @param Skey
     * @param maxlength: of skey
     * @return 0: sucess, 1 : error copying, 2: maxlength too small, 3: error getting cert
     * check ssl errors
     */
    int get_cert_HUM(char* Skey,size_t maxlength);
    /**
     * @brief get_CN_from_name
     * @param char * CN
     * @param size_t maxlength
     * @param  X509_NAME_st* certname
     * @return 0 = OK, 2 overflow, 1 error.
     */
    int get_CN_from_name(char* CN,size_t maxlength, X509_NAME2* certname);
    /**
     * @brief get_cert_CN : get CN of certificate (copy of openssl wiki)
     * @param CN : CN of certificate
     * @param maxlength : max length of CN
     * @param cert : optional cert if not using internal one
     * @return 0: OK, 1: no CN/error
     */
    int get_cert_CN(char* CN, size_t maxlength, X509 *cert=nullptr);
    /**
     * @brief get_csr_CN : get CN of csr
     * @param CN : CN of csr
     * @param maxlength : max length of CN
     * @param cert : optional csr if not using internal one
     * @return 0: OK, 1: no CN/error
     */
    int get_csr_CN(char* CN,size_t maxlength, X509_REQ* csr=nullptr);
    /**
     * @brief set_cert_PEM : load cert in skey in openssl structure
     * @param Skey
     * @param password : password if encrypted cert, can be null
     * @return 0: sucess, 1 : reading cert, 2: maxlength too small, 3: error getting cert
     * check ssl errors
     */
    int set_cert_PEM(const char* Skey, const char *password=nullptr);
    /**
     * @brief save_to_pkcs12 : save cert and key to pkcs12 file
     * @param file : opened file descriptor
     * @param name : fancy name of P12
     * @param pass : password
     * @return 0: success, 1: error creating p12 struct, 2 : error writing p12
     */
    int save_to_pkcs12(FILE* file, char *name, char *pass);
    /**
     * @brief find_friendly_name : copy of SSL Demo pkread.c to find name of pkcs12 structure
     * @param p12 : pkcs12 structure
     * @return : friendly name or NULL if not found
     */
    char *find_friendly_name(PKCS12 *p12);
    /**
     * @brief get_pkcs12_certs : put all additionnal certs of pkcs12 (in ca var) in  caList
     * @return number of certificates found.
     */
    int get_pkcs12_certs();
    /**
     * @brief load_pkcs12 : load pkcs12 from file
     * @param file : opened file desriptor
     * @param password : password to decrypt file
     * @return 0: success, 1: error reading p12, 2: error parsing p12, 3 : unsupported key type
     */
    int load_pkcs12(FILE* file, const char *password);
    /**
     * @brief get_pkcs12_name
     * @return friendly name of PKCS12 structure or NULL.
     */
    char * get_pkcs12_name();
    /**
     * @brief get_pkcs12_certs_num
     * @return num certificates decoded in pkcs12
     */
    int get_pkcs12_certs_num();
    /**
     * @brief get_pkcs12_certs_CN : get CN of cert in stack in pkcs12
     * @param n : num in ca stack
     * @return CN of cert
     */
    std::string get_pkcs12_certs_CN(unsigned int n);
    /**
     * @brief get_pkcs12_certs_pem :  get pem of cert stack in pkcs12
     * @param n : num in ca stack
     * @param Skey : returned pem
     * @param maxlength : of skeys
     * @return 0: sucess, 1 : error copying, 2: maxlength too small, 3: error getting cert, 10: n is out of bound
     */
    int get_pkcs12_certs_pem(unsigned int n,char *Skey, size_t maxlength);
    /**
     * @brief add_pkcs12_ca : add certificate in pkcs12 cert stack
     * @param Skey : certificate in pem format
     * @return 0: OK , 1: error
     */
    int add_pkcs12_ca(const char* Skey);
    /**
     * @brief create_csr
     * @return 0: sucess, 1: error
     * check ssl errors
     */
    int create_csr();
    /**
     * @brief get_csr_PEM : put CSR in PEM format to a string
     * @param Skey : string to receive CSR
     * @param maxlength : max size to put in Skey
     * @return
     */
    int get_csr_PEM(char* Skey,size_t maxlength);
    /**
     * @brief get_csr_PEM : put CSR in human readable format to a string
     * @param Skey : string to receive CSR
     * @param maxlength : max size to put in Skey
     * @return
     */
    int get_csr_HUM(char* Skey,size_t maxlength);
    /**
     * @brief set_csr_PEM : load csr in Skey in openssl structure
     * @param Skey : string containing CSR in PEM format
     * @param password : password if encrypted cert, can be null
     * @return 0: sucess, 1 : error reading csr, 2: wrong password
     * check ssl errors
     */
    int set_csr_PEM(const char* Skey, char* password);

    void set_display_callback(void(*callback)(const char*));
    void clear_display_callback();

    static int abortnow; //!< static var to abort key generation when set to 1

    /**
     * @brief print_ssl_errors : put all errors in buffer and clear them
     * @param buffer
     * @param size
     */
    void print_ssl_errors(char* buffer,size_t size);
    void empty_ssl_errors();
    int SSLError; //!< set to 1 if at least one SSL error raised. reset with empty_ssl_errors (but not with print_ssl_errors)

    // Not static const as cipher/ec list might be read from openssl in next releases.
    char digestsList[10][10] = { "sha256","sha512","sha3-256","sha3-512","shake256","sha1","md5"};//!< list of digests
    int digestsListNum=7;//!< number of digests
    char ciphersList[10][10] = { "aes256","des3","idea","des"};//!< list of ciphers
    int ciphersListNum=4;//!< number ciphers
    char keyTypeList[10][10] = { "rsa","dsa","ec"}; //!< list of key types
    int keyTypeListNum=3;//!< number of key types
    char keyECList[20][50] = {
        "prime239v3","prime256v1",
        "secp224r1","secp384r1","secp521r1",
        "sect409k1","sect409r1","sect571k1","sect571r1",
        "wap-wsg-idm-ecid-wtls10","wap-wsg-idm-ecid-wtls11"}; //!< list of ec types
    int keyECListNIDCode[20] = {
        NID_X9_62_prime239v3 ,NID_X9_62_prime256v1,
        NID_secp224r1,NID_secp384r1,NID_secp521r1,
        NID_sect409k1,NID_sect409r1,NID_sect571k1,NID_sect571r1,
        NID_wap_wsg_idm_ecid_wtls10,NID_wap_wsg_idm_ecid_wtls11}; //!< values of ec types (see obj_mach.h for full list)
    int keyECListNum=11;//!< number of keyECList/keyECListNIDCode

    /**
     * @brief set_digest : set digest for X509 cert signature
     * @param digest : one of digestsList
     * @return 0 : success, 1 error (see ssl errors)
     */
    int set_digest(char* digest);
    /**
     * @brief set_cipher : set cipher to encrypt key
     * @param cipher : one of cipherList
     * @return 0 : success, 1 error (see ssl errors)
     */
    int set_cipher(char* cipher);
    /**
     * @brief get_key_type : get string type of current loaded key in EVP
     * @param keytype : key name or "Unknown"
     * @param size : size of char*
     * @return 0 on success, 1 if unknown
     */
    int get_key_type(char* keytype, unsigned int size);
    /**
     * @brief set_X509_validity : set start and end date for X509
     * @param start : start date (YYYYMMDDHHMMSS)
     * @param end : end date (YYYYMMDDHHMMSS)
     * @return 0 : success, 1 : invalid time, 2 : end < start, 255 : unknown
     */
    int set_X509_validity(char *start, char *end);
    /**
     * @brief set_X509_serial : set serial number when signing X509
     * @param serial
     */
    void set_X509_serial(unsigned int serial);

    /* X509 v3 extentions helper */
    typedef struct x509Extension {
        char name[50]; //!< Name of extension
        int NID; //!< NID of extension
        char values[200]; //!< possible values, comma separated
    } x509Extension; //!< Structure for x509 extension array
    /* not declared as static if it can be read from openssl in future release */
    x509Extension X509ExtensionHelp[8] = {
        {"basicConstraints",NID_basic_constraints,"CA:TRUE,CA:FALSE,pathlen:<num>"},
        {"keyUsage",NID_key_usage,"digitalSignature,nonRepudiation,keyEncipherment,dataEncipherment,keyAgreement,keyCertSign,cRLSign,encipherOnly,decipherOnly"},
        {"subjectAltName",NID_subject_alt_name,"URI:http://<site>,email:<mail>,IP:<IP4/6>"},
        {"crlDistributionPoints",NID_crl_distribution_points,"URI:http://<site>"},
        {"extendedKeyUsage",NID_ext_key_usage,"serverAuth,clientAuth,codeSigning,emailProtection,timeStamping,OCSPSigning,ipsecIKE,msCodeInd,msCodeCom,msCTLSign,msEFS"},
        {"subjectKeyIdentifier",NID_subject_key_identifier,"hash"},
        {"certificatePolicies",NID_certificate_policies,"1.2.4.5"},
        {"policyConstraints",NID_policy_constraints,"requireExplicitPolicy:<num>,inhibitPolicyMapping:<num>"} //!<list of common X509v3 extensions
    };
    int X509ExtensionHelpNum=8; //!< Number of X509ExtensionHelp

    /**
     * @brief x509_extension_add  : add X509 extension
     * @param extensionNameI : name of extension
     * @param extensionValI : value
     * @param extensionCriticalI : if ext is critical
     * @return 0 on success. No failure possible for now
     */
    int x509_extension_add(std::string extensionNameI, std::string extensionValI, int extensionCriticalI);


private:

    EVP_MD* useDigest; //!< Digest to use
    EVP_CIPHER* useCipher; //!< Cypher to use
    unsigned int keyLength;//!< RSA/DSA key length
    int keyType;//!< RSA/DSA/EC type (see macro KeyRSA, KeyDSA, KeyEC)
    int keyECType;//!< Elliptic curve type by NID
    static void(*output_display)(const char*);//!< Called function to display messages when calculating keys

/* X509 subject and other options */
    char* subject_id[100]; //! < Subject id list (ex : CN, O, L...)
    unsigned char* subject[100]; //!<  Subject value list (subject in to subject_id)
    int subjectNum; //!< subject list number
    std::string extensionName [100]; //!< X509 extension name
    std::string extensionVal[100]; //!< X509 extension value
    int extensionCritical[100]; //!< X509 extension critical (1) or not (0)
    int extensionNID[100]; //!< X509 extension NID
    int extensionNum; //!< extensions num
    ASN1_TIME* startDate; //!< certificate valid from this time
    ASN1_TIME* endDate; //!< certificate valid until this time
    unsigned int serialnum; //!< x509 serial number to set
/*  Key, certs, etc... */
    X509 *x509; //!< Certificate
    EVP_PKEY *pkey; //!< private key
    RSA  *rsakey; //!< rsa key
    EC_KEY *eckey; //!< eliptic curve key
    X509_REQ *csr; //!< certificate request
    DSA *dsakey; //!< dsa key

/*  PKCS12 stuff */
    STACK_OF(X509) *ca;
    char *p12Name;
    std::vector<std::string> caListCN;
    std::vector<X509*> caListCerts;

/*  SSL Stuff */
    /**
     * @brief callback : used by ssl for display during key gen (see output_display)
     * @param p : prime num found status
     * @return
     */
    static int callback(int p, int, BN_GENCB *);
    /**
     * @brief pem_password_callback : used by ssl to get password
     * @param buf
     * @param size
     * @param userdata
     * @return
     */
    static int pem_password_callback (char *buf, int size, int /*rwflag*/, void *userdata);

    unsigned long int SSLErrorList[MAX_SSL_ERRORS]; //!< List of SSL errors when returning error codes
    int SSLErrorNum; //!< index of SSLErrorList, -1 if list is empty
    void get_ssl_errors(); //!< read and delete SSL errors in SSL lib

    BIO *bio_err; // TODO SEE if needed as declared in every functions
    char* bio_buf_error; // TODO SEE if needed (see above)

    int mkcert(X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int days);
    int add_ext(X509 *cert, int nid, const char *value);
    int add_ext_bytxt(X509 *cert, const char *nid, const char *value);

};

#endif // SSLCERTIFICATES_H
