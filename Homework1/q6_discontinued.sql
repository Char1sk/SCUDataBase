SELECT ProductName, CompanyName, ContactName
FROM Product, OrderDetail, 'Order', Customer
WHERE Discontinued == true
    and Product.Id == ProductId
    and 'Order'.Id == OrderId
    and Customer.Id == CustomerId
GROUP BY ProductName
HAVING OrderDate == min(OrderDate)
ORDER BY ProductName
;