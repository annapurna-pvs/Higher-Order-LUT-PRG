typedef unsigned char byte;


extern byte sbox_p[16];

void addroundkey_present(byte *state,byte *key);
void sbox_present(byte *state);
void keyschedule_present(byte *key,int round);
void permute_present(byte *state);

void present(byte *in,byte *out,byte *key);

void crv_present_share(byte *,int );
