SELECT Id, OrderDate, lag(OrderDate, 1, OrderDate) over(order by OrderDate) as 'A',
    round(julianday(OrderDate) - julianday(lag(OrderDate, 1, OrderDate) over(order by OrderDate)),2)
FROM 'Order'
WHERE CustomerId == 'BLONP'
LIMIT 10
;