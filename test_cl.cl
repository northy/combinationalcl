__kernel void not1(const short i1, __local short *o1) {
    *o1 = !i1;
}

__kernel void mux(__global const short *I0, __global const short *I1, __global const short *I2, __global const short *I3, __global const short *S0, __global const short *S1, __global short *O, const unsigned int n) {    
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    if (i>=n) return;

    __local short m0,m1,m2,m3,ns0,ns1,o1;
    not1(S0[i],&ns0);
    not1(S1[i],&ns1);
    and3(I0[i],ns0,ns1,&m0);
    and3(I1[i],S0[i],ns1,&m1);
    and3(I2[i],ns0,S1[i],&m2);
    and3(I3[i],S0[i],S1[i],&m3);;

    // Do the operation
    or4(m0,m1,m2,m3,&o1);
    O[i]=o1;
}
