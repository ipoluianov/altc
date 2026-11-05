extern __thread int counter;
extern __thread int libvar;
int altc_work(int n) {
    counter += n;
    libvar += n;
    return counter * 100 + libvar;
}
