#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

uint32_t bits_1;   //переменная для сдвига x^23+x^5+1
uint64_t bits_2_1; //первая переменная для полинома x^124+x^37+1
uint64_t bits_2_2; //вторая переменная для полинома x^124+x^37+1
uint64_t for_zero_first_bit_64 = 0xFFFFFFFFFFFFFFFE;
uint32_t for_zero_first_bit_32 = 0xFFFFFFFE;
void print_bits(uint8_t bits) //функция для вывода 8-ми битного числа
{
    int N(8);
    for(int i = N - 1; i >= 0; i--) {
        if(bits & ((uint8_t)1 << i))
            cout << "1";
        else
            cout << "0";
    }
    cout << endl;
}
void print_bits(uint32_t bits) //функция для вывода 32-х битного числа
{
    int N(32);
    for(int i = N - 1; i >= 0; i--) {
        if(bits & ((uint32_t)1 << i))
            cout << "1";
        else
            cout << "0";
    }
    cout << endl;
}

void print_bits(uint64_t bits) //функция для вывода 64-х битного числа
{
    int N(64);
    for(int i = N - 1; i >= 0; i--) {
        if(bits & ((uint64_t)1 << i))
            cout << "1";
        else
            cout << "0";
    }
    cout << endl;
}

uint32_t next_step_1() //функция сдвига 32-х битного числа
{
    /* С помощью смежного полинома (1+ x^-16 + x^-23) находим выходной бит,
     * далее сдвигаем биты влево на 1 и записываем на первую позицию выходной бит
     */
    uint32_t response_bit = ((bits_1 >> 15) ^ (bits_1 >> 22)) & 0x1;
    bits_1 = (((bits_1 << 1) & for_zero_first_bit_32) | response_bit);
    return response_bit;
}
uint64_t next_step_2() //функция сдвига 128-х битного числа
{
    /* Смежный полином (1+ x^-87 + x^-124). Для получения выходного бита сдвигаем второй массив
     * с битами на 87-1-64=22 и на 124-1-6=59, далее сдвигаем на 1 влево второй массив и
     * записываем последний бит первого в начало. Далее сдвигаем первый массив на 1 влево и
     * записываем в начало выходной бит.
     */
    uint64_t response_bit = ((bits_2_2 >> 22) ^ (bits_2_2 >> 59)) & 0x1;
    bits_2_2 = (((bits_2_2 << 1) & for_zero_first_bit_64) | (bits_2_1 >> 63));
    bits_2_1 = (((bits_2_1 << 1) & for_zero_first_bit_64) | response_bit);
    return response_bit;
}
uint8_t mask_byte(uint8_t byte) //маскирование байта
{

    uint8_t response_byte = 0;
    for(int i = 7; i >= 0; i--) {
        uint32_t x1 = next_step_1();
        uint32_t x2 = next_step_2();
        uint8_t x = x1 ^ x2;
        if(i != 0) {
            response_byte = ((((byte >> i) ^ x)) | response_byte) << 1;
        } else {
            response_byte = ((((byte >> i) ^ x)) | response_byte);
        }
    }
    return response_byte;
}
int check_num(uint8_t byte, bool zero) //подсчёт числа нулей и единиц 
{
    int num_z = 0;
    int num_o = 0;

    for(int i = 7; i >= 0; i--) {
        if(byte & ((uint8_t)1 << i))
            num_o += 1;
        else
            num_z += 1;
    }
    if(zero)
        return num_z;
    else
        return num_o;
}
void mask_file() //маскирование файла
{
    ifstream file_1("/home/user/Projects_C/Project_1/Project_1/Project_1/1.txt", ios::binary | ios::in);
    ofstream file_2("/home/user/Projects_C/Project_1/Project_1/Project_1/1_mask.txt", ios::binary | ios::out);
    int zero_out = 0, one_out = 0, zero_in = 0,
        one_in = 0; // переменные для подсчёта проведения частотного тестирования
    if(!file_1) {
        cout << "Файл не найден" << endl;
        return;
    }
    file_1.seekg(0, ios::end);
    //    int size_file_1 = file_1.tellg();
    file_1.seekg(0, ios::beg);
    char byte_out;
    while(file_1.get(byte_out)) {
        zero_out += check_num((uint8_t)byte_out, 1);
        one_out += check_num((uint8_t)byte_out, 0);
        uint8_t byte_in = mask_byte((uint8_t)byte_out);
        zero_in += check_num((uint8_t)byte_in, 1);
        one_in += check_num((uint8_t)byte_in, 0);
        file_2.write((char*)&byte_in, sizeof(char));
    }
    file_1.close();
    file_2.close();
    double a = (erfc(abs(zero_out - one_out) / pow(zero_out + one_out, 0.5))) / pow(2, 0.5);
    double b = (erfc(abs(zero_in - one_in) / pow(zero_in + one_in, 0.5))) / pow(2, 0.5);
    cout << "Частотный тест битов файла до маскирования:" << a << endl;
    cout << "Частотный тест битов файла после маскирования:" << b << endl;
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    bits_1 = rand(); // генерация массива битов
    bits_2_1 = ((uint64_t)rand() << 32) | rand();
    bits_2_2 = ((uint64_t)rand() << 32) | rand();
    cout << "Сколько чисел сгенерировать?" << endl;
    int num;
    cin >> num;
    int zero = 0, one = 0;
    for(int i = 0; i < num; i++) {
        uint32_t x1 = next_step_1();
        uint32_t x2 = next_step_2();
        uint32_t x = x1 ^ x2;

        if(x) {
            one += 1;
            cout << "1";
        } else {
            zero += 1;
            cout << "0";
        }
    }
    cout << endl;
    double a = (erfc(abs(zero - one) / pow(zero + one, 0.5))) / pow(2, 0.5);
    cout << "Частотный тест:" << a << endl;
    cout << "Маскирование файла 1.txt >> 1_mask.txt" << endl;
    mask_file();
    return 0;
}
