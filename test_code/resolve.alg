program resolve
begin
    var a := "global";
    var b := "yurt";
    procedure show() begin
        println a+b;
    end
    procedure ex() begin
        let a := "inner";
        let b := "bloop";
        show();
        println a+b;
    end
    show();
    ex();
end.