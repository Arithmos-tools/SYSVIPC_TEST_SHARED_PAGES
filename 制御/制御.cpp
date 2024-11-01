/**
 * \file 制御.cpp
 * \brief 掛け算を行うためのソフトウェア
 *
 *
 *
 * \author Arithmos-Tools
 * \date 令和六年十一月二日
 */

//! 定数文字列の長さを返す
/*!
  \return 文字列の長さ (定数)
*/
constexpr unsigned long long 文字列の長さ(const char *str)
{
  unsigned long long count = 0;
  while (*str != 0x00)
  {
    ++count;
    ++str;
  }
  return count;
}

//! 文字列の長さを返す
/*!
  \return 文字列の長さ 
*/
unsigned long long 文字列の長さ(char *str)
{
  unsigned long long count = 0;
  while (*str != 0x00)
  {
    ++count;
    ++str;
  }
  return count;
}

//! システムコールを呼んで文字列を表示する (システムコール 0x01)
/*!
  \param 文字列　
*/
template<typename T>
inline void 表示(T 文字列)
{
  asm volatile("movq $0x01, %%rdi\n\t"
               "movq $0x01, %%rax\n\t"
               "movq %0, %%rsi\n\t"
               "movq %1, %%rdx\n\t"
               "syscall\n\t"
               :
               : "r"(文字列), "r"(文字列の長さ(文字列))
               : "%rdi", "%rax", "%rsi", "%rdx");
}
//! プログラムを終了する
/*!
  \param エラー (エラーコード)
*/
inline void 終了(signed long long エラー)
{
  asm volatile("movq %0, %%rdi\n\t"
               "movq $0x3C, %%rax\n\t"
               "syscall\n\t"
               :
               : "r"(エラー)
               : "%rdi", "%rax");
}

//! 共有メモリ領域を新規作成、または既存の領域を取得します。
/*!
  \return 共有メモリ領域の識別子
  \sa shmat()
*/
inline signed long long shmget()
{
  signed long long 戻り値;
  asm volatile("movq $123456789, %%rdi\n\t"
               "movq $0x1D, %%rax\n\t"
               "movq $4096, %%rsi\n\t"
               "movq %1, %%rdx\n\t"
               "syscall\n\t"
               "movq %%rax, %0\n\t"
               : "=r"(戻り値)
               : "r"((unsigned long long)(512 | 0666))
               : "%rdi", "%rax", "%rsi", "%rdx");
  if (戻り値 < 0)
  {
    表示("Cant get the shared page id\n");
    終了(戻り値);
  }
  return 戻り値;
}

//! メモリ領域の識別子を引数に取り、アドレスを返す
/*!
  \param 識別子
  \return アドレス
  \sa shmget()
*/
inline signed long long shmat(signed long long 識別子)
{
  signed long long 戻り値 = 0;
  asm volatile("movq %1, %%rdi\n\t"
               "movq $0x1E, %%rax\n\t"
               "movq $0, %%rsi\n\t"
               "movq $0, %%rdx\n\t"
               "syscall\n\t"
               "movq %%rax, %0\n\t"
               : "=r"(戻り値)
               : "r"(識別子)
               : "%rdi", "%rax", "%rsi", "%rdx");
  if (reinterpret_cast<signed long long>(戻り値) <= 0)
  {
    表示("Cant get the shared page address\n");
    終了(reinterpret_cast<signed long long>(戻り値));
  }
  return 戻り値;
}

//! sched_yieldというシステムコールを呼ぶ
/*!
 */
void 待つ()
{
  asm volatile("movq $0x19, %%rax\n\t"
               "syscall\n\t"
               :
               :
               :);
}

//! 掛け算用のソフトを実行する (fork)
/*!
 */
signed long long 掛け算を行う()
{
  signed long long 戻り値 = 0;
  asm volatile("movq $0x39, %%rax\n\t"
               "syscall\n\t"
               "movq %%rax, %0\n\t"
               : "=r"(戻り値)
               :);
  return 戻り値;
}

struct __kernel_timespec
{
  unsigned long long tv_sec;
  unsigned long long tv_nsec;
};

int main()
{
  signed long long 領域 = shmget();
  int *配列 = reinterpret_cast<int *>(shmat(領域));
  // 行列の幅 : 3
  // 行列の高さ : 3
  // 結果は３つ目

  /*  １つ目の行列　
   *   |1|2|3|
   *   |4|5|6|
   *   |7|8|9|
   */

  /*  2つ目の行列　
   *   |10|11|12|
   *   |13|14|15|
   *   |16|17|18|
   */
  for (unsigned char i = 1; i < 19; ++i) // 行列の初期化
    配列[i - 1] = i;
  配列[27] = 0;
  unsigned char セルの識別子 = 0;
  signed long long プレセス識別子 = 0;
  signed long long 識別子配列[9];
  for (セルの識別子 = 0; セルの識別子 < 9; ++セルの識別子)
  {
    プレセス識別子 = 掛け算を行う();
    if (プレセス識別子 < 0)
    {
      表示("Error while calling fork\n");
      終了(-1);
    }
    else if (プレセス識別子 == 0)
      break;
    識別子配列[セルの識別子] = プレセス識別子;
  }
  int 状態[9];
  if (プレセス識別子 == 0)
  {
    配列[セルの識別子 + 18] = 0;
    char 数字[1];
    表示("Executing process for cell number ");
    数字[0] = セルの識別子+'0';
    表示(数字);
    表示("\n");
    for (int i = 0; i < 3; ++i)
    {
      配列[セルの識別子 + 18] += 配列[3 * int(セルの識別子 / 3) + i] * 配列[3 * i + ((セルの識別子) % 3) + 9];
    }
    asm(
        "lock; inc %0\n\t"
        :
        : "m"(配列[27])
        : "memory");
  }
  else
  {
    while (配列[27] != 9)
      待つ(); // sched_yield
    表示("All processes finished successfully\n");
  }
  if (プレセス識別子 != 0)
  {
    if (配列[18] == 84 &&
        配列[19] == 90 &&
        配列[20] == 96 &&
        配列[21] == 201 &&
        配列[22] == 216 &&
        配列[23] == 231 &&
        配列[24] == 318 &&
        配列[25] == 342 &&
        配列[26] == 366)
    {
      表示("Correct result\n");
      return 0;
    }
    else
    {
      表示("Incorrect result\n");
      return -1;
    }
  }
  else
    return 0;
}