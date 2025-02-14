program counter
begin
    let i := 1;
    procedure A() 
    begin
        let j := 1;
        procedure B()
        begin
            j := j + 1;
            return j;
        end
        return B();
    end
    let x := A();
    while (i < 5) do
    begin
        println x();
        i := i + 1;
    end
end.