#!/usr/local/bin/dalgol -v
program fibonacci
begin
    let cnt := 0;
    procedure fibo(var n)
    begin
        if (n <= 1) then
        begin
            return 1;
        end
            else
        begin
            return fibo(n-2)+fibo(n-1);
        end
    end
    while (cnt < 20) do
    begin
        println fibo(cnt);
        cnt := cnt + 1;
    end
end.