program resolve
begin
    var a := "global";
    var b := "yurt";
    procedure show() begin
        println a+b;
    end
    procedure ex() begin
        show();
        let a := "inner";
        let b := "bloop";
        show();
    end
    ex();
    println a+b;
end.