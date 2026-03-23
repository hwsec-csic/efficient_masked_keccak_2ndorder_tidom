#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include "hdf5.h"



/*************************************************************DEFINICIONES DE DATOS Y DECLARACIONES DE FUNCIONES*************************************************************/
#ifndef KECCAKF_ROUNDS
#define KECCAKF_ROUNDS 24
#endif

#ifndef ROTL64
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))
#endif

typedef struct {
    union {                                 // state:
        uint8_t b[200];                     // 8-bit bytes
        uint64_t q[25];                     // 64-bit words
    } st;
    int pt, rsiz, mdlen;                    // these don't overflow
} keccak_ctx_t;

uint8_t bytes_st[25][8];

void keccakf(uint64_t st[25]);

int sha3_init(keccak_ctx_t *c, int mdlen);    // mdlen = hash output in bytes
int sha3_update(keccak_ctx_t *c, const void *data, size_t len);
int sha3_final(void *md, keccak_ctx_t *c);    // digest goes to md

void *sha3(const void *in, size_t inlen, void *md, int mdlen);


int trace_counter;
const char *h5_name = "INTERMEDIATE_VALUES_2ND_ORDER.h5";

/*************************************************************DEFINICIONES DE FUNCIONES*************************************************************/

int create_state_dataset(const char *filename)
{
    hid_t file_id;
    hid_t dataspace_id_s0;
    hid_t dataspace_id_s1;
    hid_t dataspace_id_s2;
    hid_t dataspace_id_s;
    hid_t dataset_id_s0;
    hid_t dataset_id_s1;
    hid_t dataset_id_s2;
    hid_t dataset_id_s;

    hsize_t dims[3] = {100, 24, 25};

    // Abrir archivo existente en modo lectura/escritura
    file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);

    // Crear dataspace con dimensiones 10x24x64
    dataspace_id_s0 = H5Screate_simple(3, dims, NULL);
    dataspace_id_s1 = H5Screate_simple(3, dims, NULL);
    dataspace_id_s2 = H5Screate_simple(3, dims, NULL);
    dataspace_id_s = H5Screate_simple(3, dims, NULL);

    // Crear dataset llamado "state_s0"
    dataset_id_s0 = H5Dcreate2(
        file_id,
        "state_s0",
        H5T_NATIVE_UINT64,
        dataspace_id_s0,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT
    );
    if (dataset_id_s0 < 0) {
        printf("Error creando dataset\n");
    }

    dataset_id_s1 = H5Dcreate2(
        file_id,
        "state_s1",
        H5T_NATIVE_UINT64,
        dataspace_id_s1,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT
    );

    if (dataset_id_s1 < 0) {
        printf("Error creando dataset\n");
    }

    dataset_id_s2 = H5Dcreate2(
        file_id,
        "state_s2",
        H5T_NATIVE_UINT64,
        dataspace_id_s2,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT
    );

    if (dataset_id_s2 < 0) {
        printf("Error creando dataset\n");
    }

    dataset_id_s = H5Dcreate2(
        file_id,
        "state_unmasked",
        H5T_NATIVE_UINT64,
        dataspace_id_s,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT
    );

    if (dataset_id_s < 0) {
        printf("Error creando dataset\n");
    }

    
    // Cerrar recursos
    H5Dclose(dataset_id_s0);
    H5Dclose(dataset_id_s1);
    H5Dclose(dataset_id_s2);
    H5Dclose(dataset_id_s);
    H5Sclose(dataspace_id_s0);
    H5Sclose(dataspace_id_s1);
    H5Sclose(dataspace_id_s2);
    H5Sclose(dataspace_id_s);
    H5Fclose(file_id);

    return 0;
}



int write_value(const char *filename,
                      const char *field,
                      uint64_t value,
                      int i, int j, int k)
{
    hid_t file_id, dataset_id;
    hid_t filespace, memspace;

    hsize_t offset[3] = {i, j, k};
    hsize_t count[3]  = {1, 1, 1};

    file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
    if (file_id < 0) return -1;


    dataset_id = H5Dopen2(file_id, field, H5P_DEFAULT);
    if (dataset_id < 0) {
        H5Fclose(file_id);
        return -1;
    }

    filespace = H5Dget_space(dataset_id);

    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

    memspace = H5Screate_simple(3, count, NULL);

    H5Dwrite(dataset_id,
             H5T_NATIVE_UINT64,
             memspace,
             filespace,
             H5P_DEFAULT,
             &value);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(dataset_id);
    H5Fclose(file_id);

    return 0;
}


int read_value(const char *filename,
                        const char *field,
                        uint8_t *value,
                        int i, int j, int k)
{
    hid_t file_id, dataset_id;
    hid_t filespace, memspace;

    hsize_t offset[3] = {i, j, k};
    hsize_t count[3]  = {1, 1, 1};

    // Abrir archivo
    file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) return -1;

    // Abrir dataset
    dataset_id = H5Dopen2(file_id, field, H5P_DEFAULT);
    if (dataset_id < 0) {
        H5Fclose(file_id);
        return -1;
    }

    // Obtener dataspace del dataset
    filespace = H5Dget_space(dataset_id);

    // Seleccionar el elemento (i,j,k)
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

    // Crear espacio de memoria para 1 elemento
    memspace = H5Screate_simple(3, count, NULL);

    // Leer valor
    if (H5Dread(dataset_id,
                H5T_NATIVE_UINT8,
                memspace,
                filespace,
                H5P_DEFAULT,
                value) < 0)
    {
        H5Sclose(memspace);
        H5Sclose(filespace);
        H5Dclose(dataset_id);
        H5Fclose(file_id);
        return -1;
    }

    // Cerrar recursos
    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(dataset_id);
    H5Fclose(file_id);

    return 0;
}


// ***********************************************************************
// ******************************* CHACHA20 ******************************
// ***********************************************************************

#define CHACHA20_BLOCK_SIZE 64
#define SEED_SIZE 32


int seed_counter = 0;
uint32_t prng_state;
uint8_t seed_chacha20[32];

typedef struct {
    uint32_t state[16];
    uint8_t keystream[CHACHA20_BLOCK_SIZE];
    int keystream_pos;
} chacha20_prng_t;

static chacha20_prng_t prng;
static const uint32_t chacha20_constants[4] = {
    0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
};

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QUARTERROUND(a,b,c,d) \
    a += b; d ^= a; d = ROTL(d,16); \
    c += d; b ^= c; b = ROTL(b,12); \
    a += b; d ^= a; d = ROTL(d,8);  \
    c += d; b ^= c; b = ROTL(b,7);

static void chacha20_block(uint32_t out[16], const uint32_t in[16]) {
    uint32_t x[16];
    memcpy(x, in, sizeof(uint32_t) * 16);

    for (int i = 0; i < 10; i++) {
        QUARTERROUND(x[0], x[4], x[8], x[12])
        QUARTERROUND(x[1], x[5], x[9], x[13])
        QUARTERROUND(x[2], x[6], x[10], x[14])
        QUARTERROUND(x[3], x[7], x[11], x[15])

        QUARTERROUND(x[0], x[5], x[10], x[15])
        QUARTERROUND(x[1], x[6], x[11], x[12])
        QUARTERROUND(x[2], x[7], x[8], x[13])
        QUARTERROUND(x[3], x[4], x[9], x[14])
    }

    for (int i = 0; i < 16; i++) {
        out[i] = x[i] + in[i];
    }
}

void prng_init(const uint8_t seed[32]) {
    prng.state[0] = chacha20_constants[0];
    prng.state[1] = chacha20_constants[1];
    prng.state[2] = chacha20_constants[2];
    prng.state[3] = chacha20_constants[3];

    for (int i = 0; i < 8; i++) {
        prng.state[4 + i] =
            ((uint32_t)seed[i*4]) |
            ((uint32_t)seed[i*4+1] << 8) |
            ((uint32_t)seed[i*4+2] << 16) |
            ((uint32_t)seed[i*4+3] << 24);
    }

    prng.state[12] = 0; // counter
    prng.state[13] = 0; prng.state[14] = 0; prng.state[15] = 0; // nonce

    prng.keystream_pos = CHACHA20_BLOCK_SIZE;
}

static void prng_generate_block() {
    chacha20_block((uint32_t*)prng.keystream, prng.state);
    prng.state[12]++;
    prng.keystream_pos = 0;
}

uint8_t prng_get_byte() {
    if (prng.keystream_pos >= CHACHA20_BLOCK_SIZE) {
        prng_generate_block();
    }
    return prng.keystream[prng.keystream_pos++];
}

uint64_t rand_share_64() {
    uint64_t r = 0;
    for (int i = 0; i < 8; i++) {
        r |= ((uint64_t)prng_get_byte()) << (8 * i);
    }
    return r;
}


// ***********************************************************************
// ******************************* KECCAK ******************************
// ***********************************************************************


void keccakf(uint64_t st[25])
{
    int i, j, r, MSB, LSB;
    const uint64_t keccakf_rndc[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
        0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
        0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
        0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
        0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
        0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
        0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };

    uint64_t keccakf_rndc_1[24], keccakf_rndc_2[24], keccakf_rndc_3[24];
    for(i=0;i<24;i++)
    {  
        keccakf_rndc_1[i] = rand_share_64();
        keccakf_rndc_2[i] = rand_share_64();
        keccakf_rndc_3[i] = keccakf_rndc_1[i] ^ keccakf_rndc_2[i] ^ keccakf_rndc[i];
    }

    const int keccakf_rotc[24] = {
        1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
        27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
    };

    const int keccakf_piln[24] = {
        10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
        15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
    };

    #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    uint8_t *v;

    // endianess conversion. this is redundant on little-endian targets
    for (i = 0; i < 25; i++) {
        v = (uint8_t *) &st[i];
        st[i] = ((uint64_t) v[0])     | (((uint64_t) v[1]) << 8) |
            (((uint64_t) v[2]) << 16) | (((uint64_t) v[3]) << 24) |
            (((uint64_t) v[4]) << 32) | (((uint64_t) v[5]) << 40) |
            (((uint64_t) v[6]) << 48) | (((uint64_t) v[7]) << 56);
    }
    #endif

    // GENERACIÓN DE SHARES
    uint64_t st_1[25];
    uint64_t st_2[25];
    uint64_t st_3[25];

    uint64_t mask_1;
    uint64_t mask_2;
    uint64_t mask_3;

    for(i=0;i<25;i++)
    {   
        mask_1 = rand_share_64();
        st_1[i] = mask_1;

        mask_2 = rand_share_64();
        st_2[i] = mask_2;

        mask_3 = mask_1 ^ mask_2;
        st_3[i] = mask_3 ^ st[i];    
    }



    // KECCAK CON ENMASCARAMIENTO DE 3 SHARES
    uint64_t t_1, t_2, t_3, bc_1[5], bc_2[5], bc_3[5];
    for (r = 0; r < KECCAKF_ROUNDS; r++)
    {
        printf("--- RONDA ACTUAL: %i ---\n\n\n",r);
        
        //%%%%%%%%%PASO 1. Theta%%%%%%%%%
        for (i = 0; i < 5; i++)
        {
        bc_1[i] = st_1[i] ^ st_1[i + 5] ^ st_1[i + 10] ^ st_1[i + 15] ^ st_1[i + 20];
        bc_2[i] = st_2[i] ^ st_2[i + 5] ^ st_2[i + 10] ^ st_2[i + 15] ^ st_2[i + 20];
        bc_3[i] = st_3[i] ^ st_3[i + 5] ^ st_3[i + 10] ^ st_3[i + 15] ^ st_3[i + 20];
        }

        for (i = 0; i < 5; i++) 
        { 
            t_1 = bc_1[(i + 4) % 5] ^ ROTL64(bc_1[(i + 1) % 5], 1);
            t_2 = bc_2[(i + 4) % 5] ^ ROTL64(bc_2[(i + 1) % 5], 1); 
            t_3 = bc_3[(i + 4) % 5] ^ ROTL64(bc_3[(i + 1) % 5], 1); 
            for (j = 0; j < 25; j += 5)
            {
                st_1[j + i] ^= t_1; 
                st_2[j + i] ^= t_2;
                st_3[j + i] ^= t_3;
            }
        }

        

        printf("Salida de Theta: \n\n");
        for (int i = 0; i < 25; i++) {
            MSB=(i+1)*64-1;
            LSB=64*i;
            printf("S_theta_out_1[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_1[i]);
            printf("S_theta_out_2[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_2[i]);
            printf("S_theta_out_3[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_3[i]);
            printf("\n\n");
        }
        printf("\n\n\n");
        

        //%%%%%%%%%PASO 2+3. Rho+Pi%%%%%%%%%

        t_1 = st_1[1];     
        t_2 = st_2[1];
        t_3 = st_3[1];             

        for (i = 0; i < 24; i++) 
        {  
            j = keccakf_piln[i];  

            bc_1[0] = st_1[j];  
            bc_2[0] = st_2[j];
            bc_3[0] = st_3[j];
            
            st_1[j] = ROTL64(t_1, keccakf_rotc[i]);
            st_2[j] = ROTL64(t_2, keccakf_rotc[i]);
            st_3[j] = ROTL64(t_3, keccakf_rotc[i]);
            
            t_1 = bc_1[0];   
            t_2 = bc_2[0];   
            t_3 = bc_3[0];             
        }

        printf("Salida de Rho+Pi: \n\n");
        for (int i = 0; i < 25; i++) {
            MSB=(i+1)*64-1;
            LSB=64*i;
            printf("S_rho_pi_out_1[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_1[i]);
            printf("S_rho_pi_out_2[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_2[i]);
            printf("S_rho_pi_out_3[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_3[i]);
            printf("\n\n");
        }
        printf("\n\n\n");



        //%%%%%%%%%PASO 4. Chi%%%%%%%%%

        for (j = 0; j < 25; j += 5) 
        {  
            for (i = 0; i < 5; i++)
            {    
                bc_1[i] = st_1[j + i];
                bc_2[i] = st_2[j + i];  
                bc_3[i] = st_3[j + i];
            }      
            for (i = 0; i < 5; i++)
            {     
                st_1[j + i] = bc_2[i] ^ bc_2[(i+2)%5] ^ (bc_2[(i+1)%5] & bc_2[(i+2)%5]) ^ (bc_2[(i+1)%5] & bc_3[(i+2)%5]) ^ (bc_3[(i+1)%5] & bc_2[(i+2)%5]);
                st_2[j + i] = bc_3[i] ^ bc_3[(i+2)%5] ^ (bc_3[(i+1)%5] & bc_3[(i+2)%5]) ^ (bc_3[(i+1)%5] & bc_1[(i+2)%5]) ^ (bc_1[(i+1)%5] & bc_3[(i+2)%5]);
                st_3[j + i] = bc_1[i] ^ bc_1[(i+2)%5] ^ (bc_1[(i+1)%5] & bc_1[(i+2)%5]) ^ (bc_1[(i+1)%5] & bc_2[(i+2)%5]) ^ (bc_2[(i+1)%5] & bc_1[(i+2)%5]);
            } 
        }

        printf("Salida de Chi: \n\n");
        for (int i = 0; i < 25; i++) {
            MSB=(i+1)*64-1;
            LSB=64*i;
            printf("S_chi_out_1[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_1[i]);
            printf("S_chi_out_2[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_2[i]);
            printf("S_chi_out_3[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_3[i]);
            printf("\n\n");
        }
        printf("\n\n\n");

        //%%%%%%%%%PASO 5. Iota%%%%%%%%%
        st_1[0] ^= keccakf_rndc_1[r];  
        st_2[0] ^= keccakf_rndc_2[r];  
        st_3[0] ^= keccakf_rndc_3[r];  

        printf("Salida de Iota: \n\n");
        for (int i = 0; i < 25; i++) {
            MSB=(i+1)*64-1;
            LSB=64*i;
            printf("S_iota_out_1[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_1[i]);
            printf("S_iota_out_2[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_2[i]);
            printf("S_iota_out_3[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)st_3[i]);
            printf("\n\n");
        }
        printf("\n\n\n");

        for(int i=0; i < 25; i++)
        {
            write_value(h5_name, "state_s0", st_1[i], trace_counter, r, i);
            write_value(h5_name, "state_s1", st_2[i], trace_counter, r, i);
            write_value(h5_name, "state_s2", st_3[i], trace_counter, r, i);
        }

        //%%%%%%%%%Reconstrucción del estado - Desenmascarado%%%%%%%%%
        for(i=0;i<25;i++)
        {
            st[i] = st_1[i] ^ st_2[i] ^ st_3 [i];
            for(j=0;j<8;j++)
            {
                bytes_st[i][j] = ((st[i] >> (j * 8)) & 0xFF);
            }
            write_value(h5_name, "state_unmasked", st[i], trace_counter, r, i);
        }

        // printf("Salida descompuesta por bytes:\n\n");

        for (int i = 0; i < 25; i++) {
            for (int j = 0; j < 8; j++) {
                MSB = i*64+((j+1)*8-1);
                LSB = MSB-7;
                // printf("S_out[%4d:%4d] = 8'h_%02x;\n", MSB, LSB, bytes_st[i][j]);
            }
        }
        // printf("\n\n\n");
    }

    #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    // endianess conversion. this is redundant on little-endian targets
    for (i = 0; i < 25; i++) {
        v = (uint8_t *) &st[i];
        t = st[i];
        v[0] = t & 0xFF;
        v[1] = (t >> 8) & 0xFF;
        v[2] = (t >> 16) & 0xFF;
        v[3] = (t >> 24) & 0xFF;
        v[4] = (t >> 32) & 0xFF;
        v[5] = (t >> 40) & 0xFF;
        v[6] = (t >> 48) & 0xFF;
        v[7] = (t >> 56) & 0xFF;
        }
    #endif
    
}

int sha3_init(keccak_ctx_t *c, int mdlen)  
{
    int i;

    for (i = 0; i < 25; i++)
        c->st.q[i] = 0; 
    c->mdlen = mdlen;  
    c->rsiz = 200 - 2 * mdlen; 
    c->pt = 0; 

    return 1;  
}

int sha3_update(keccak_ctx_t *c, const void *data, size_t len)
{
    size_t i;
    int j;

    //int MSB,LSB;

    j = c->pt;
    for (i = 0; i < len; i++) {     
        c->st.b[j++] ^= ((const uint8_t *) data)[i]; 
        if (j >= c->rsiz) 
        {        
            
            /*
            printf("Estado de entrada a Keccak: \n\n");
            for (int i = 0; i < 25; i++) {
                MSB=(i+1)*64-1;
                LSB=64*i;
                printf("S_in_theta[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)c->st.q[i]);
            }

            printf("\n\n\n");
            */
            keccakf(c->st.q);
            j = 0;
        }
    }
    c->pt = j;  
                
    return 1;
}

// finalize and output a hash

int sha3_final(void *md, keccak_ctx_t *c)
{   
    int i;

    c->st.b[c->pt] ^= 0x06; 
    c->st.b[c->rsiz - 1] ^= 0x80; 
   
    /*
    printf("Estado de entrada a Keccak del ÚLTIMO MENSAJE ABSORBIDO: \n\n");
    for (int i = 0; i < 25; i++) {
        MSB=(i+1)*64-1;
        LSB=64*i;
        printf("S_in_theta[%2d:%2d] = 64'h_%016llx;\n", MSB, LSB, (unsigned long long)c->st.q[i]);
    }

    printf("\n\n\n");
    */   

    keccakf(c->st.q);  

    for (i = 0; i < c->mdlen; i++) {
        ((uint8_t *) md)[i] = c->st.b[i];
    }

    return 1;   
}   

void *sha3(const void *in, size_t inlen, void *md, int mdlen)
{
    keccak_ctx_t sha3;
    printf("Llamada a función sha-3 correcta\r\n");
    sha3_init(&sha3, mdlen);
    sha3_update(&sha3, in, inlen);
    sha3_final(md, &sha3);

    return md;
}

/*************************************************************------    MAIN    ------*************************************************************/

int main(int argc, char **argv)
{
   

    create_state_dataset(h5_name);

    int input_len = 16;
    uint8_t input[16];


    for(trace_counter=0; trace_counter<100; trace_counter++)
    {

    
        for(int i=0; i<16; i++)
        {
            read_value(h5_name, "plaintexts", &input[i], trace_counter, 0, i);
        }

        for(int i=0; i<32; i++)
        {
            read_value(h5_name, "seeds", &seed_chacha20[i], trace_counter, 0, i);
        }

        prng_init(seed_chacha20);

        int length_SHA3 = 64;
        uint8_t hash_sha3[length_SHA3]; 

        sha3(input, input_len, hash_sha3, length_SHA3); 

        printf("SHA3-512(");
        for (int i = 0; i < input_len; i++) {
            printf("%02x", input[i]);
        if (i < input_len - 1) printf(" ");
        }
        printf(") = ");

        for (int i = 0; i < length_SHA3; i++)
            printf("%02x", hash_sha3[i]);
        printf("\n");




    }


    

    

    

    

    
    
    return 0;
}