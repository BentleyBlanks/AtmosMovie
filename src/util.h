
template<typename T>
int getNumOfDigits(const T& number)
{
    int count = 1;
    T temp = number;

    while((int)(temp / 10) != 0)
    {
        temp = (int) (temp / 10);
        count++;
    }

    return count;
}
