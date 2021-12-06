SELECT distinct ShipName, substr(ShipName, 1, instr(ShipName, '-')-1)
FROM 'Order'
WHERE ShipName like "%-%"
ORDER BY ShipName;
-- 'instr' find the index of substr
-- 'substr' get substring
-- || to concate strings