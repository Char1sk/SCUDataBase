SELECT Id, ShipCountry, iif(ShipCountry in ('USA', 'Mexico', 'Canada'), 'NorthAmerica', 'OtherPlace')
FROM 'Order'
WHERE Id >= 15445
LIMIT 20;