program list
begin 
    let arr[15];
    let t := 0;
    let cnt := 0;
    procedure addToBack(var k) 
    begin 
        arr[t] := k;
        t := t + 1;
    end
    procedure printList() 
    begin 
        cnt := 0;
        while (cnt < t) do
        begin 
            println arr[cnt];
            cnt := cnt + 1;
        end
    end
    {* main procedure starts here *}
    while (cnt < 15) do
    begin 
        addToBack(2*cnt+1);
        cnt := cnt + 1;
    end
    printList();
end.