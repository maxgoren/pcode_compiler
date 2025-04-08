program factorial
begin
    procedure fact(var n)
    begin
        if (n < 2) then 
            begin
                return 1;    
            end
        else
            begin 
                return fact(n-1)*n;
            end
    end
    let x := 3;
    while (x < 10) do begin
        println fact(x);
        x++;
    end
end.