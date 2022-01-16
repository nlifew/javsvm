
public class Native {

    public int add(int a, int b) { return a + b; }

    public static native int nAdd(int a, int b);

    public static native double nAdd(
        boolean z, byte b, char c, short s,
        int i, long j, float f, double d,
        boolean z1, byte b1, char c1, short s1,
        int i1, long j1, float f1, double d1
    );
}