SELECT CompanyName, round(100.0 * count(iif(ShippedDate > RequiredDate,1,NULL)) / count(*), 2) as per
FROM 'Order', Shipper
WHERE ShipVia == Shipper.Id
GROUP BY ShipVia
ORDER BY per DESC
;