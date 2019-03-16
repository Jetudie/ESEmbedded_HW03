// pass two integers
int gcd(int a, int b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

// pass multiple paramter
int add(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

// pass an array
int sum(int a[6])
{
    int sum = 0;

    for (int i = 0; i < 6; i++)
        sum += a[i];

    return sum;
}

void reset_handler(void)
{
    int a = 24, b = 18;
    int c;

    // pass two integers
    c = gcd(a, b);

    // pass six integers
    c = add(1, 2, 3, a, b, c);

    int x[6] = { 4, 5, 6, a, b, c };
    // pass an array
    c = sum(x);

    while (1)
        ;
}
