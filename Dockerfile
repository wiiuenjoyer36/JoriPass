FROM python:3

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN apt install -y libpq-dev
RUN pip install --no-cache-dir -r requirements.txt

COPY server .
COPY version.env .

EXPOSE 8080

CMD [ "python", "./server.py" ]
