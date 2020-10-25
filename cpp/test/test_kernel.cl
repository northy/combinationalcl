__kernel void combinational(int ic, __global const uchar* inputs, int oc, __global uchar* outputs) {
    int time = get_global_id(0);
    int id = get_global_id(1);
    outputs[time*oc+id]=inputs[time*ic+id]*(1+id)*(1+time);
}
