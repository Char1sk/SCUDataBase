SELECT CategoryName, count(*), round(avg(UnitPrice),2), min(UnitPrice), max(UnitPrice), sum(UnitsOnOrder)
FROM Product, Category
WHERE CategoryId == Category.Id
GROUP BY CategoryId
ORDER BY CategoryId

;