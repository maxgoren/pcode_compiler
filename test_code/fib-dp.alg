#!/usr/local/bin/dalgol -v
program fibonacci
begin
    let cache[100];
    let cnt := 0;
    {* dynamic programming version of recursive fibonacci algorithm. uses an array to save previously computed results *}
    procedure fibo(var n)
    begin
        if (n <= 1) then
        begin
            return 1;
        end
            else
        begin
            if (cache[n] == 0) then
            begin
                cache[n] := fibo(n-2)+fibo(n-1);
            end
            return cache[n];
        end
    end
    while (cnt < 30) do
    begin
        println cnt + ": " + fibo(cnt);
        cnt := cnt + 1;
    end
    println fib(6);
end.