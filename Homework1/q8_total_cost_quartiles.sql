SELECT  CompanyName, CustomerId, ROUND(cost,2)
FROM
    (SELECT IFNULL(CompanyName, "MISSING_NAME") as CompanyName, CustomerId, cost, ntile(4) OVER(ORDER BY CAST(cost as float)) as quartile 
    FROM 
        (SELECT "Order".CustomerId, SUM((UnitPrice*Quantity)) as cost
        FROM "Order"
        LEFT OUTER JOIN OrderDetail ON "Order".Id = OrderDetail.OrderId
        GROUP BY "Order".CustomerId)
    LEFT OUTER JOIN Customer ON CustomerId = Customer.Id)
WHERE quartile==1
ORDER BY CAST(cost as float);