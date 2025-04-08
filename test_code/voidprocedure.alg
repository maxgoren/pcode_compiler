program testprog
begin
    record listnode
    begin
        var info;
        var next;
    end
    let node := new listnode;
    let tmp := new listnode;
    node.info := 13;
    node.next := tmp;
    tmp.info := 42;
    tmp.next := 0;
    println node;
    println node.info;
    println tmp;
    println tmp.info;
end.